/*!*********************************************************************************************************************
\file         PVRAssets\FileIO\PFXReader.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Implementation of methods of the PFXReader class.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRAssets/FileIO/PFXReader.h"
#include "PVRCore/StringFunctions.h"
#include "PVRAssets/PixelFormat.h"
#include "PVRCore/FileStream.h"
#include "PVRCore/Assert_.h"
#include <cstring>
#include <cstdio>
#include <cstdlib>

namespace {
using namespace pvr;
using namespace pvr::types;
const char8* g_LinearStr       = "LINEAR";
const char8* g_NearestStr = "NEAREST";
const char8* g_NoneStr = "NONE";
const char8* g_ClampStr = "CLAMP";
const char8* g_RepeatStr = "REPEAT";
const char8* g_MirrorRepeatStr = "MIRROR_REPEAT";
const char8* g_BorderStr = "BORDER";
const char8* g_MirrorClampStr = "MIRROR_CLAMP";
const char8* g_CurrentViewStr = "PFX_CURRENTVIEW";
const uint32 g_PfxTexColor = uint32(1 << 30);
const uint32 g_PfxTexDepth = uint32(1 << 31);



const char8* Filters[(uint32)SamplerFilter::Size] =
{
	g_NearestStr,		// eFilter_Nearest
	g_LinearStr,		// FilterLinear
	g_NoneStr,			// FilterNone
};
const char8* Wraps[(uint32)SamplerWrap::Size] =
{
	g_RepeatStr,
	g_MirrorRepeatStr,
	g_ClampStr,
	g_BorderStr,
	g_MirrorClampStr
};

const uint32 g_DfltViewPortWidth = 640;
const uint32 g_DfltViewPortHeight = 480;

#define NEWLINE_TOKENS "\r\n"
#define DELIM_TOKENS " \t"

bool getSemanticDataFromString(assets::EffectSemanticData& pDataItem, const char8* const pszArgumentString,
                               types::SemanticDataType eType, std::string& errorOut)
{
	char8* pszString = (char8*)pszArgumentString;
	char8* pszTmp;
	const assets::EffectSemanticDefaultDataTypeInfo& sDfltType =
	  assets::EffectSemanticDefaultDataTypeInfo::getSemanticDefaultTypeInfo(eType);
	strings::ignoreWhitespace(&pszString);

	if (pszString[0] != '(')
	{
		errorOut = std::string("Missing '(' after ") + sDfltType.name;
		return false;
	}
	pszString++;

	strings::ignoreWhitespace(&pszString);

	if (!strlen(pszString))
	{
		errorOut = sDfltType.name + std::string(" missing arguments");
		return false;
	}

	pszTmp = pszString;
	switch (sDfltType.internalType)
	{
	case types::EffectDefaultDataInternalType::Float:
		pDataItem.dataF32[0] = (float)strtod(pszString, &pszTmp);
		break;
	case types::EffectDefaultDataInternalType::Integer:
		pDataItem.dataI32[0] = (int32)strtol(pszString, &pszTmp, 10);
		break;
	case types::EffectDefaultDataInternalType::Boolean:
		if (strncmp(pszString, "true", 4) == 0)
		{
			pDataItem.dataBool[0] = true;
			pszTmp = &pszString[4];
		}
		else if (strncmp(pszString, "false", 5) == 0)
		{
			pDataItem.dataBool[0] = false;
			pszTmp = &pszString[5];
		}
		break;
	}

	if (pszString == pszTmp)
	{
		size_t n = strcspn(pszString, ",\t ");
		errorOut.assign("'");
		errorOut.append(pszString, n);
		errorOut.append("' unexpected for ");
		errorOut.append(sDfltType.name);
		return false;
	}
	pszString = pszTmp;

	strings::ignoreWhitespace(&pszString);

	for (uint32 i = 1; i < sDfltType.numDataItems; i++)
	{
		if (!strlen(pszString))
		{
			errorOut = sDfltType.name + std::string(" missing arguments");
			return false;
		}

		if (pszString[0] != ',')
		{
			size_t n = strcspn(pszString, ",\t ");
			errorOut.assign("'");
			errorOut.append(pszString, n);
			errorOut.append("' unexpected for ");
			errorOut.append(sDfltType.name);
			return false;
		}
		pszString++;

		strings::ignoreWhitespace(&pszString);

		if (!strlen(pszString))
		{
			errorOut = sDfltType.name + std::string(" missing arguments");
			return false;
		}

		pszTmp = pszString;
		switch (types::EffectDefaultDataInternalType(sDfltType.internalType))
		{
		case types::EffectDefaultDataInternalType::Float:
			pDataItem.dataF32[i] = (float)strtod(pszString, &pszTmp);
			break;
		case types::EffectDefaultDataInternalType::Integer:
			pDataItem.dataI32[i] = (int32)strtol(pszString, &pszTmp, 10);
			break;
		case types::EffectDefaultDataInternalType::Boolean:
			if (strncmp(pszString, "true", 4) == 0)
			{
				pDataItem.dataBool[i] = true;
				pszTmp = &pszString[4];
			}
			else if (strncmp(pszString, "false", 5) == 0)
			{
				pDataItem.dataBool[i] = false;
				pszTmp = &pszString[5];
			}
			break;
		}

		if (pszString == pszTmp)
		{
			size_t n = strcspn(pszString, ",\t ");
			errorOut.assign("'");
			errorOut.append(pszString, n);
			errorOut.append("' unexpected for ");
			errorOut.append(sDfltType.name);
			return false;
		}
		pszString = pszTmp;

		strings::ignoreWhitespace(&pszString);
	}

	if (pszString[0] != ')')
	{
		size_t n = strcspn(pszString, ",\t ");
		errorOut.assign("'");
		errorOut.append(pszString, n);
		errorOut.append("' unexpected for ");
		errorOut.append(sDfltType.name);
		return false;
	}
	pszString++;

	strings::ignoreWhitespace(&pszString);

	if (strlen(pszString))
	{
		errorOut = std::string("'") + pszString + "' unexpected after ')'";
		return false;
	}

	return true;
}

}

namespace pvr {
namespace assets {

class PFXParserReadContext
{
public:
	std::vector<string> ppszEffectFile;
	std::vector<int32> fileLineNumbers;
	uint32	nNumLines;

public:
	PFXParserReadContext()
	{
		nNumLines = 0;
	}
};

PfxParserEffect::PfxParserEffect()/* :
	Uniforms(DEFAULT_EFFECT_NUM_UNIFORM),
	Attributes(DEFAULT_EFFECT_NUM_ATTRIB),
	textures(DEFAULT_EFFECT_NUM_TEX)*/
{
}

PfxRenderPass::PfxRenderPass() :
	renderPassType(EffectPassType::Null),
	viewType(EffectPassView::None),
	formatFlags(0),
	effect(NULL),
	texture(NULL)
{
}

PfxReader::PfxReader(): m_fileName(""), m_viewportWidth(g_DfltViewPortWidth), m_viewportHeight(g_DfltViewPortHeight), m_currentEffect(0)
{
}

PfxReader::~PfxReader()
{
	for (std::vector<PFXParserTexture*>::iterator it = m_textures.begin(); it != m_textures.end(); ++it)
	{
		delete *it; *it = 0;
	}
}

bool PfxReader::parse(string& pReturnError)
{
	enum Cmd
	{
		CmdsHeader,
		CmdsTexture,
		CmdsTarget,
		CmdsTextures,
		CmdsVertexShader,
		CmdsFragmentShader,
		CmdsEffect,

		CmdsSize
	};

	const StringHash parserCommands[] =
	{
		StringHash("[HEADER]"),				// eCmds_Header
		StringHash("[TEXTURE]"),			// eCmds_Texture
		StringHash("[TARGET]"),				// eCmds_Target
		StringHash("[TEXTURES]"),			// eCmds_Textures
		StringHash("[VERTEXSHADER]"),		// eCmds_VertexShader
		StringHash("[FRAGMENTSHADER]"),		// eCmds_FragmentShader
		StringHash("[EFFECT]"),				// eCmds_Effect
	};
	PVR_STATIC_ASSERT(sizeof(parserCommands) / sizeof(parserCommands[0]) == CmdsSize, parserCommands);

	int32 nEndLine = 0;
	int32 nHeaderCounter = 0, nTexturesCounter = 0;
	uint32 i, j, k;

	// Loop through the file
	for (uint32 nLine = 0; nLine < m_context->nNumLines; nLine++)
	{
		// Skip blank lines
		if (m_context->ppszEffectFile[nLine].empty()) {continue;}

		StringHash Cmd(m_context->ppszEffectFile[nLine]);
		if (Cmd ==  parserCommands[CmdsHeader])
		{
			if (nHeaderCounter > 0)
			{
				pReturnError = strings::createFormatted("[HEADER] redefined on line %d\n", m_context->fileLineNumbers[nLine]);
				return false;
			}
			if (getEndTag("HEADER", nLine, &nEndLine))
			{
				if (parseHeader(nLine, nEndLine, pReturnError))
				{
					nHeaderCounter++;
				}
				else
				{
					return false;
				}
			}
			else
			{
				pReturnError = strings::createFormatted("Missing [/HEADER] tag after [HEADER] on line %d\n", m_context->fileLineNumbers[nLine]);
				return false;
			}
			nLine = nEndLine;
		}
		else if (Cmd == parserCommands[CmdsTexture])
		{
			if (getEndTag("TEXTURE", nLine, &nEndLine))
			{
				if (!parseTexture(nLine, nEndLine, pReturnError))
				{
					return false;
				}
			}
			else
			{
				pReturnError = strings::createFormatted("Missing [/TEXTURE] tag after [TEXTURE] on line %d\n", m_context->fileLineNumbers[nLine]);
				return false;
			}
			nLine = nEndLine;
		}
		else if (Cmd == parserCommands[CmdsTarget])
		{
			if (getEndTag("TARGET", nLine, &nEndLine))
			{
				if (!parseTarget(nLine, nEndLine, pReturnError))
				{
					return false;
				}
			}
			else
			{
				pReturnError = strings::createFormatted("Missing [/TARGET] tag after [TARGET] on line %d\n", m_context->fileLineNumbers[nLine]);
				return false;
			}
			nLine = nEndLine;
		}
		else if (Cmd == parserCommands[CmdsTextures])
		{
			if (nTexturesCounter > 0)
			{
				pReturnError = strings::createFormatted("[TEXTURES] redefined on line %d\n", m_context->fileLineNumbers[nLine]);
				return false;
			}
			if (getEndTag("TEXTURES", nLine, &nEndLine))
			{
				if (parseTextures(nLine, nEndLine, pReturnError))
				{
					nTexturesCounter++;
				}
				else
				{
					return false;
				}
			}
			else
			{
				pReturnError = strings::createFormatted("Missing [/TEXTURES] tag after [TEXTURES] on line %d\n",
				                                        m_context->fileLineNumbers[nLine]);
				return false;
			}
			nLine = nEndLine;
		}
		else if (Cmd == parserCommands[CmdsVertexShader])
		{
			if (getEndTag("VERTEXSHADER", nLine, &nEndLine))
			{
				PFXParserShader vertexShader;
				if (parseShader(nLine, nEndLine, pReturnError, vertexShader, "VERTEXSHADER"))
				{
					m_vertexShaders.push_back(vertexShader);
				}
				else
				{
					return false;
				}
			}
			else
			{
				pReturnError = strings::createFormatted("Missing [/VERTEXSHADER] tag after [VERTEXSHADER] on line %d\n",
				                                        m_context->fileLineNumbers[nLine]);
				return false;
			}
			nLine = nEndLine;
		}
		else if (Cmd == parserCommands[CmdsFragmentShader])
		{
			if (getEndTag("FRAGMENTSHADER", nLine, &nEndLine))
			{
				PFXParserShader fragShader;
				if (parseShader(nLine, nEndLine, pReturnError, fragShader, "FRAGMENTSHADER"))
				{
					m_fragmentShaders.push_back(fragShader);
				}
				else
				{
					return false;
				}
			}
			else
			{
				pReturnError = strings::createFormatted("Missing [/FRAGMENTSHADER] tag after [FRAGMENTSHADER] on line %d\n",
				                                        m_context->fileLineNumbers[nLine]);
				return false;
			}
			nLine = nEndLine;
		}
		else if (Cmd == parserCommands[CmdsEffect])
		{
			if (getEndTag("EFFECT", nLine, &nEndLine))
			{
				PfxParserEffect effect;
				if (parseEffect(effect, nLine, nEndLine, pReturnError))
				{
					m_effects.push_back(effect);
				}
				else
				{
					return false;
				}
			}
			else
			{
				pReturnError = strings::createFormatted("Missing [/EFFECT] tag after [EFFECT] on line %d\n", m_context->fileLineNumbers[nLine]);
				return false;
			}
			nLine = nEndLine;
		}
		else
		{
			pReturnError = strings::createFormatted("'%s' unexpected on line %d\n", m_context->ppszEffectFile[nLine].c_str(),
			                                        m_context->fileLineNumbers[nLine]);
			return false;
		}
	}

	if (m_effects.size() < 1)
	{
		pReturnError = string("No [EFFECT] found. PFX file must have at least one defined.\n");
		return false;
	}

	if (m_fragmentShaders.size() < 1)
	{
		pReturnError = string("No [FRAGMENTSHADER] found. PFX file must have at least one defined.\n");;
		return false;
	}

	if (m_vertexShaders.size() < 1)
	{
		pReturnError = string("No [VERTEXSHADER] found. PFX file must have at least one defined.\n");
		return false;
	}

	// Loop Effects
	for (i = 0; i < m_effects.size(); ++i)
	{
		// Loop textures in Effects
		for (j = 0; j < m_effects[i].textures.size(); ++j)
		{
			// Loop textures in whole PFX
			uint32 uiTexSize = (uint32)m_textures.size();
			for (k = 0; k < uiTexSize; ++k)
			{
				if (m_textures[k]->name == m_effects[i].textures[j].name)
				{
					break;
				}
			}

			// Texture mismatch. Report error.
			if (!uiTexSize || k == uiTexSize)
			{
				pReturnError = "Error: TEXTURE '" + m_effects[i].textures[j].name.str() + "' is not defined in [TEXTURES].\n";
				return false;
			}
		}
	}

	determineRenderPassDependencies(pReturnError);
	if (pReturnError.compare(""))
	{
		return false;
	}

	return true;
}

bool PfxReader::parseFromMemory(const char8* const pszScript, string& pReturnError)
{
	PFXParserReadContext	context;
	char8			pszLine[512];
	const char8*		pszEnd, *pszCurr;
	int32				nLineCounter;
	uint32	nLen;
	uint32	nReduce;
	bool			bDone;

	if (!pszScript)
	{
		Log(Log.Error, "[PfxReader::parseFromMemory] Attempted to parse from NULL pointer.");
		return false;
	}

	m_context = &context;

	// Find & process each line
	nLineCounter	= 0;
	bDone			= false;
	pszCurr			= pszScript;
	while (!bDone)
	{
		nLineCounter++;

		while (*pszCurr == '\r')
		{
			++pszCurr;
		}

		// Find length of line
		pszEnd = strchr(pszCurr, '\n');
		if (pszEnd)
		{
			nLen = (uint32)(pszEnd - pszCurr);
		}
		else
		{
			nLen = (uint32)strlen(pszCurr);
			bDone = true;
		}

		nReduce = 0; // Tells how far to go back because of '\r'.
		while (nLen - nReduce > 0 && pszCurr[nLen - 1 - nReduce] == '\r')
		{
			nReduce++;
		}

		// Ensure pszLine will not be not overrun
		if (nLen + 1 - nReduce > sizeof(pszLine) / sizeof(*pszLine))
		{
			nLen = sizeof(pszLine) / sizeof(*pszLine) - 1 + nReduce;
		}

		// Copy line into pszLine
		strncpy(pszLine, pszCurr, nLen - nReduce);
		pszLine[nLen - nReduce] = 0;
		pszCurr += nLen + 1;

		assertion(strchr(pszLine, '\r') == 0);
		assertion(strchr(pszLine, '\n') == 0);

		// Ignore comments
		char8* tmp = strstr(pszLine, "//");
		if (tmp != NULL)	{ *tmp = '\0'; }

		// Reduce whitespace to one character.
		reduceWhitespace(pszLine);

		// Store the line, even if blank lines (to get correct errors from GLSL compiler).
		m_context->fileLineNumbers.push_back(nLineCounter);
		m_context->ppszEffectFile.push_back(pszLine);
		m_context->nNumLines++;
	}

	return parse(pReturnError);
}

bool PfxReader::parseFromFile(Stream::ptr_type pfxFile, string& errorOut)
{

	if (!pfxFile.get() || !pfxFile->isopen())
	{
		errorOut = string("[PfxReader::parseFromFile]: Unable to open file ") + (pfxFile.get() ? pfxFile->getFileName().c_str() : "");
		return false;
	}
	string PfxFileString;
	std::vector<char8>pfxData;
	pfxData.resize(pfxFile->getSize() + 1);
	size_t sizeRead;
	bool rslt = pfxFile->read(pfxFile->getSize(), 1, &pfxData[0], sizeRead);
	if (!rslt) {return rslt; }
	// Is our shader resource file data null terminated?
	if (pfxData[pfxFile->getSize()] != '\0')
	{
		// If not create a temporary null-terminated string
		pfxData[pfxFile->getSize()] = '\0';
		PfxFileString.assign(&pfxData[0]);
		pfxData.assign(PfxFileString.begin(), PfxFileString.end());
	}

	m_fileName = pfxFile->getFileName();

	return parseFromMemory(&pfxData[0], errorOut);
}

bool PfxReader::setViewportSize(uint32 width, uint32 height)
{
	if (width > 0 && height > 0)
	{
		m_viewportWidth = width;
		m_viewportHeight = height;
		return true;
	}
	else
	{
		return false;
	}
}

bool PfxReader::retrieveRenderPassDependencies(std::vector<PfxRenderPass*>& aRequiredRenderPasses,
    std::vector<StringHash>& aszActiveEffecstrings)
{
	uint32 ui(0), uj(0), uk(0), ul(0);
	const PfxParserEffect* pTempEffect(NULL);

	if (aRequiredRenderPasses.size() > 0)
	{
		/* aRequiredRenderPasses should be empty when it is passed in */
		return false;
	}

	for (ui = 0; ui < (uint32)aszActiveEffecstrings.size(); ++ui)
	{
		if (aszActiveEffecstrings[ui].length())
		{
			// Empty strings are not valid
			return false;
		}

		// Find the specified effect
		for (uj = 0, pTempEffect = NULL; uj < (uint32)m_effects.size(); ++uj)
		{
			if (aszActiveEffecstrings[ui] == m_effects[uj].name)
			{
				// Effect found
				pTempEffect = &m_effects[uj];
				break;
			}
		}

		if (pTempEffect == NULL)
		{
			// Effect not found
			return false;
		}

		for (uj = 0; uj < m_renderPassSkipGraph.getNumNodes(); ++uj)
		{
			if (m_renderPassSkipGraph[uj]->effect == pTempEffect)
			{
				m_renderPassSkipGraph.RetreiveSortedDependencyList(aRequiredRenderPasses, uj);
				return true;
			}
		}

		/*
			The effect wasn't a post-process. Check to see if it has any non-post-process dependencies,
			e.g. RENDER CAMERA textures.
		*/
		// Loop Effects
		for (uj = 0; uj < (uint32)m_effects.size(); ++uj)
		{
			if (aszActiveEffecstrings[ui] != m_effects[uj].name)
			{
				continue;
			}

			// Loop textures in Effect
			for (uk = 0; uk < m_effects[uj].textures.size(); ++uk)
			{
				// Loop Render Passes for whole PFX
				for (ul = 0; ul < m_renderPasses.size(); ++ul)
				{
					// Check that the name of this render pass output texture matches a provided texture in an Effect
					if (m_renderPasses[ul].texture->name == m_effects[uj].textures[uk].name)
					{
						aRequiredRenderPasses.push_back(&m_renderPasses[ul]);
					}
				}
			}

			return true;
		}
	}

	return false;
}

bool PfxReader::getEndTag(const char8* pszTagName, int32 nStartLine, int32* pnEndLine)
{
	char8 pszEndTag[100];
	strcpy(pszEndTag, "[/");
	strcat(pszEndTag, pszTagName);
	strcat(pszEndTag, "]");
	for (uint32 i = nStartLine; i < m_context->nNumLines; i++)
	{
		if (strcmp(pszEndTag, m_context->ppszEffectFile[i].c_str()) == 0)
		{
			*pnEndLine = i;
			return true;
		}
	}

	return false;
}

void PfxReader::reduceWhitespace(char8* line)
{
	// convert tabs and newlines to ' '
	char8* tmp = strpbrk(line, "\t\n");
	while (tmp != NULL)
	{
		*tmp = ' ';
		tmp = strpbrk(line, "\t\n");
	}

	// remove all whitespace at start
	while (line[0] == ' ')
	{
		// move chars along to omit whitespace
		int32 counter = 0;
		do
		{
			line[counter] = line[counter + 1];
			counter++;
		}
		while (line[counter] != '\0');
	}

	// step through chars of line remove multiple whitespace
	for (int32 i = 0; i < (int32)strlen(line); i++)
	{
		// whitespace found
		if (line[i] == ' ')
		{
			// count number of whitespace chars
			int32 numWhiteChars = 0;
			while (line[i + 1 + numWhiteChars] == ' ')
			{
				numWhiteChars++;
			}

			// multiple whitespace chars found
			if (numWhiteChars > 0)
			{
				// move chars along to omit whitespace
				int32 counter = 1;
				while (line[i + counter] != '\0')
				{
					line[i + counter] = line[i + numWhiteChars + counter];
					counter++;
				}
			}
		}
	}

	// If there is no string then do not remove terminating white symbols
	if (!strlen(line))
	{
		return;
	}

	// remove all whitespace from end
	while (line[strlen(line) - 1] == ' ')
	{
		// move chars along to omit whitespace
		line[strlen(line) - 1] = '\0';
	}
}

string PfxReader::findParameter(char8* aszSourceString, const string& parameterTag, const string& delimiter)
{
	string returnString("");
	char8* aszTagStart = strstr(aszSourceString, parameterTag.c_str());

	// Tag was found, so search for parameter
	if (aszTagStart)
	{
		char8* aszDelimiterStart = strstr(aszTagStart, delimiter.c_str());
		char8* aszSpaceStart = strstr(aszTagStart, " ");

		// Delimiter found
		if (aszDelimiterStart && (!aszSpaceStart || (aszDelimiterStart < aszSpaceStart)))
		{
			// Create a string from the delimiter to the next space
			size_t strCount(strcspn(aszDelimiterStart, " "));
			aszDelimiterStart++;	// Skip =
			returnString.assign(aszDelimiterStart, strCount - 1);
		}
	}

	return returnString;
}

bool PfxReader::readStringToken(char8* pszSource, string& output, string& ErrorStr, int32 i, const char8* pCaller)
{
	if (*pszSource == '\"')		// Quote marks. Continue parsing until end mark or NULL
	{
		pszSource++;		// Skip past first quote
		while (*pszSource != '\"')
		{
			if (*pszSource == '\0')
			{
				ErrorStr = strings::createFormatted("Incomplete argument in [%s] on line %d: %s\n", pCaller, m_context->fileLineNumbers[i],
				                                    m_context->ppszEffectFile[i].c_str());
				return false;
			}

			output.push_back(*pszSource);
			pszSource++;
		}

		pszSource++;		// Skip past final quote.
	}
	else		// No quotes. Read until space
	{
		pszSource = strtok(pszSource, DELIM_TOKENS NEWLINE_TOKENS);
		output = pszSource;

		pszSource += strlen(pszSource);
	}

	// Check that there's nothing left on this line
	pszSource = strtok(pszSource, NEWLINE_TOKENS);
	if (pszSource)
	{
		ErrorStr = strings::createFormatted("Unknown keyword '%s' in [%s] on line %d: %s\n", pszSource, pCaller,
		                                    m_context->fileLineNumbers[i],  m_context->ppszEffectFile[i].c_str());
		return false;
	}

	return true;
}

bool PfxReader::parseHeader(int32 nStartLine, int32 nEndLine, string& pReturnError)
{
	enum eCmd
	{
		CmdsVersion,
		CmdsDescription,
		CmdsCopyright,
		CmdsSize
	};

	const StringHash HeaderCommands[] =
	{
		StringHash("VERSION"),			// eCmds_Version
		StringHash("DESCRIPTION"),		// eCmds_Description
		StringHash("COPYRIGHT"),		// eCmds_Copyright
	};
	PVR_STATIC_ASSERT(sizeof(HeaderCommands) / sizeof(HeaderCommands[0]) == CmdsSize, HeaderCommands);

	for (int32 i = nStartLine + 1; i < nEndLine; i++)
	{
		// Skip blank lines
		if (m_context->ppszEffectFile[i].empty()) {	continue;	}

		char8* str = strtok(const_cast<char8*>(m_context->ppszEffectFile[i].c_str()), " ");
		if (str != NULL)
		{
			StringHash Cmd(str);
			if (Cmd == HeaderCommands[CmdsVersion])
			{
				str += (strlen(str) + 1);
				m_header.Version = str;
			}
			else if (Cmd == HeaderCommands[CmdsDescription])
			{
				str += (strlen(str) + 1);
				m_header.Description = str;
			}
			else if (Cmd == HeaderCommands[CmdsCopyright])
			{
				str += (strlen(str) + 1);
				m_header.Copyright = str;
			}
			else
			{
				pReturnError = strings::createFormatted("Unknown keyword '%s' in [HEADER] on line %d\n", str, m_context->fileLineNumbers[i]);
				return false;
			}
		}
		else
		{
			pReturnError = strings::createFormatted("Missing arguments in [HEADER] on line %d : %s\n", m_context->fileLineNumbers[i],
			                                        m_context->ppszEffectFile[i].c_str());
			return false;
		}
	}

	return true;
}


template<typename EnumType>
static bool parseTextureFlags(const char8* c_pszRemainingLine, EnumType** ppFlagsOut, uint32 uiNumFlags,
                              const char8** c_ppszFlagNames, uint32 uiNumFlagNames,
                              string& returnError, int32 iLineNum, PFXParserReadContext* m_context)
{
	const EnumType INVALID_TYPE = EnumType(0xAC1DBEEF);
	uint32 uiIndex;
	const char8* c_pszCursor;
	const char8* c_pszResult;

	// --- Find the first flag
	uiIndex = 0;
	c_pszCursor = strstr(c_pszRemainingLine, c_ppszFlagNames[uiIndex++]);
	while (uiIndex < uiNumFlagNames)
	{
		c_pszResult = strstr(c_pszRemainingLine, c_ppszFlagNames[uiIndex++]);
		if (((c_pszResult < c_pszCursor) || !c_pszCursor) && c_pszResult)
		{
			c_pszCursor = c_pszResult;
		}
	}

	if (!c_pszCursor)
	{
		return true;  // No error, but just return as no flags specified.
	}

	// Quick error check - make sure that the first flag found is valid.
	if (c_pszCursor != c_pszRemainingLine)
	{
		if (*(c_pszCursor - 1) == '-')		// Yeah this shouldn't be there. Must be invalid first tag.
		{
			char8 szBuffer[128];		// Find out the tag.
			memset(szBuffer, 0, sizeof(szBuffer));
			const char8* pszStart = c_pszCursor - 1;
			while (pszStart != c_pszRemainingLine && *pszStart != ' ') { pszStart--; }
			pszStart++;	// Escape the space.
			uint32 uiNumChars = (uint32)((c_pszCursor - 1) - pszStart);
			strncpy(szBuffer, pszStart, uiNumChars);

			returnError = strings::createFormatted("Unknown keyword '%s' in [TEXTURES] on line %d: %s\n", szBuffer,
			                                       m_context->fileLineNumbers[iLineNum], m_context->ppszEffectFile[iLineNum].c_str());
			return false;
		}
	}

	uint32 uiFlagsFound = 0;
	uint32 uiBufferIdx;
	char8 szBuffer[128];		// Buffer to hold the token

	while (*c_pszCursor != ' ' && *c_pszCursor != 0 && uiFlagsFound < uiNumFlags)
	{
		memset(szBuffer, 0, sizeof(szBuffer));		// Clear the buffer
		uiBufferIdx = 0;

		while (*c_pszCursor != '-' && *c_pszCursor != 0 && *c_pszCursor != ' ' && uiBufferIdx < 128)		// - = delim. token
		{
			szBuffer[uiBufferIdx++] = *c_pszCursor++;
		}

		// Check if the buffer content is a valid flag name.
		EnumType Type = INVALID_TYPE;
		for (uint32 uiIndex = 0; uiIndex < uiNumFlagNames; ++uiIndex)
		{
			if (strcmp(szBuffer, c_ppszFlagNames[uiIndex]) == 0)
			{
				Type = static_cast<EnumType>(uiIndex); // Yup, it's valid. uiIndex here would translate to one of the enums that matches the string array of flag names passed in.
				break;
			}
		}

		// Tell the user it's invalid.
		if (Type == INVALID_TYPE)
		{
			returnError = strings::createFormatted("Unknown keyword '%s' in [TEXTURES] on line %d: %s\n", szBuffer,
			                                       m_context->fileLineNumbers[iLineNum], m_context->ppszEffectFile[iLineNum].c_str());
			return false;
		}

		// Set the flag to the enum type.
		*ppFlagsOut[uiFlagsFound++] = Type;

		if (*c_pszCursor == '-')	{ c_pszCursor++; }
	}

	return true;
}

bool PfxReader::parseGenericSurface(int32 nStartLine, int32 nEndLine, PFXParserTexture& Params,
                                    std::vector<StringHash>& KnownCmds,
                                    const char8* pCaller, string& pReturnError)
{
	const uint32 INVALID_TYPE = 0xAC1DBEEF;

	enum eCmd
	{
		eCmds_Min,
		eCmds_Mag,
		eCmds_Mip,
		eCmds_WrapS,
		eCmds_WrapT,
		eCmds_WrapR,
		eCmds_Filter,
		eCmds_Wrap,
		eCmds_Resolution,
		eCmds_Surface,

		eCmds_Size
	};

	const StringHash GenericSurfCommands[] =
	{
		StringHash("MINIFICATION"),			// eCmds_Min
		StringHash("MAGNIFICATION"),		// eCmds_Mag
		StringHash("MIPMAP"),				// eCmds_Mip
		StringHash("WRAP_S"),				// eCmds_WrapS
		StringHash("WRAP_T"),				// eCmds_WrapT
		StringHash("WRAP_R"),				// eCmds_WrapR
		StringHash("FILTER"),				// eCmds_Filter
		StringHash("WRAP"),					// eCmds_Wrap
		StringHash("RESOLUTION"),			// eCmds_Resolution
		StringHash("SURFACETYPE"),			// eCmds_Surface
	};
	PVR_STATIC_ASSERT(sizeof(GenericSurfCommands) / sizeof(GenericSurfCommands[0]) == eCmds_Size, GenericSurfCommands);

	struct SSurfacePair
	{
		StringHash name;
		PixelFormat eType;
		uint32 BufferType;
	};

	const SSurfacePair surfacePairs[] =
	{
		{ StringHash("RGBA8888"),	PixelFormat::RGBA_8888, g_PfxTexColor },
		{ StringHash("RGBA4444"),	PixelFormat::RGBA_4444, g_PfxTexColor },
		{ StringHash("RGB888"),		PixelFormat::RGB_888, g_PfxTexColor },
		{ StringHash("RGB565"),		PixelFormat::RGB_565, g_PfxTexColor },
		{ StringHash("INTENSITY8"), PixelFormat::Intensity8, g_PfxTexColor },
		{ StringHash("DEPTH24"),	PixelFormat::Depth24, g_PfxTexDepth },
		{ StringHash("DEPTH16"),	PixelFormat::Depth16, g_PfxTexDepth },
		{ StringHash("DEPTH8"),		PixelFormat::Depth8, g_PfxTexDepth },
	};
	const uint32 numSurfaceType = sizeof(surfacePairs) / sizeof(surfacePairs[0]);

	for (int32 i = nStartLine + 1; i < nEndLine; i++)
	{
		// Skip blank lines
		if (m_context->ppszEffectFile[i].empty()) {	continue; }

		// Need to make a copy so we can use strtok and not affect subsequent parsing
		string blockCopy(m_context->ppszEffectFile[i]);
		char8* str = strtok(const_cast<char8*>(blockCopy.c_str()), NEWLINE_TOKENS DELIM_TOKENS);
		if (!str) {	return false; }

		StringHash Cmd(str);
		const char8** ppParams    = NULL;
		uint32 uiNumParams = 0;
		bool bKnown = false;

		// --- Verbose filtering flags
		if (Cmd == GenericSurfCommands[eCmds_Min] || Cmd == GenericSurfCommands[eCmds_Mag] || Cmd == GenericSurfCommands[eCmds_Mip])
		{
			ppParams    = Filters;
			uiNumParams = (uint32)SamplerFilter::Size;
			bKnown      = true;
		}
		// --- Verbose wrapping flags
		else if (Cmd == GenericSurfCommands[eCmds_WrapS] || Cmd == GenericSurfCommands[eCmds_WrapT]
		         || Cmd == GenericSurfCommands[eCmds_WrapR])
		{
			ppParams    = Wraps;
			uiNumParams = (uint32)SamplerWrap::Size;
			bKnown      = true;
		}
		// --- Inline filtering flags
		else if (Cmd == GenericSurfCommands[eCmds_Filter])
		{
			char8* pszRemaining = strtok(NULL, NEWLINE_TOKENS DELIM_TOKENS);
			if (!pszRemaining)
			{
				pReturnError = strings::createFormatted("Missing FILTER arguments in [%s] on line %d: %s\n", pCaller,
				                                        m_context->fileLineNumbers[i],  m_context->ppszEffectFile[i].c_str());
				return false;
			}

			SamplerFilter* pFlags[3] =
			{
				&Params.minFilter,
				&Params.magFilter,
				&Params.mipFilter,
			};

			if (!parseTextureFlags<SamplerFilter>(pszRemaining, pFlags, 3, Filters, (uint32)SamplerFilter::Size, pReturnError, i, m_context)) { return false; }
			bKnown     = true;
		}
		// --- Inline wrapping flags
		else if (Cmd == GenericSurfCommands[eCmds_Wrap])
		{
			char8* pszRemaining = strtok(NULL, NEWLINE_TOKENS DELIM_TOKENS);
			if (!pszRemaining)
			{
				pReturnError = strings::createFormatted("Missing WRAP arguments in [%s] on line %d: %s\n", pCaller, m_context->fileLineNumbers[i],
				                                        m_context->ppszEffectFile[i].c_str());
				return false;
			}

			SamplerWrap* pFlags[3] =
			{
				&Params.wrapS,
				&Params.wrapT,
				&Params.wrapR,
			};

			if (!parseTextureFlags<SamplerWrap>(pszRemaining, pFlags, 3, Wraps, (uint32)SamplerWrap::Size, pReturnError, i, m_context)) { return false; }
			bKnown     = true;
		}
		// --- Resolution
		else if (Cmd == GenericSurfCommands[eCmds_Resolution])
		{
			char8* pszRemaining;

			uint32* uiVals[2] = { &Params.width, &Params.height };

			// There should be precisely TWO arguments for resolution (width and height)
			for (uint32 uiIndex = 0; uiIndex < 2; ++uiIndex)
			{
				pszRemaining = strtok(NULL, DELIM_TOKENS NEWLINE_TOKENS);
				if (!pszRemaining)
				{
					pReturnError = strings::createFormatted("Missing RESOLUTION argument(s) (requires width AND height) in [TARGET] on line %d\n",
					                                        m_context->fileLineNumbers[i]);
					return false;
				}

				int32 val = atoi(pszRemaining);

				if ((val == 0
				     && *pszRemaining != '0')			// Make sure they haven't explicitly set the value to be 0 as this might be a valid use-case.
				    || (val < 0))
				{
					pReturnError = strings::createFormatted("Invalid RESOLUTION argument \"%s\" in [TEXTURE] on line %d\n", pszRemaining,
					                                        m_context->fileLineNumbers[i]);
					return false;
				}

				*(uiVals[uiIndex]) = (uint32)val;
			}

			bKnown     = true;
		}
		// --- Surface type
		else if (Cmd == GenericSurfCommands[eCmds_Surface])
		{
			char8* pszRemaining = strtok(NULL, NEWLINE_TOKENS DELIM_TOKENS);
			if (!pszRemaining)
			{
				pReturnError = strings::createFormatted("Missing SURFACETYPE arguments in [TARGET] on line %d\n", m_context->fileLineNumbers[i]);
				return false;
			}

			StringHash hashType(pszRemaining);
			for (uint32 uiIndex = 0; uiIndex < numSurfaceType; ++uiIndex)
			{
				if (hashType == surfacePairs[uiIndex].name)
				{
					Params.flags =  surfacePairs[uiIndex].eType.getPixelTypeId() | surfacePairs[uiIndex].BufferType;
					break;
				}
			}

			bKnown     = true;
		}

		// Valid Verbose command
		if (ppParams)
		{
			char8* pszRemaining = strtok(NULL, NEWLINE_TOKENS DELIM_TOKENS);
			if (!pszRemaining)
			{
				pReturnError = strings::createFormatted("Missing arguments in [%s] on line %d: %s\n", pCaller, m_context->fileLineNumbers[i],
				                                        m_context->ppszEffectFile[i].c_str());
				return false;
			}

			uint32 Type = INVALID_TYPE;
			for (uint32 uiIndex = 0; uiIndex < uiNumParams; ++uiIndex)
			{
				if (strcmp(pszRemaining, ppParams[uiIndex]) == 0)
				{
					Type = uiIndex;			// Yup, it's valid.
					break;
				}
			}

			// Tell the user it's invalid.
			if (Type == INVALID_TYPE)
			{
				pReturnError = strings::createFormatted("Unknown keyword '%s' in [%s] on line %d: %s\n", pszRemaining, pCaller,
				                                        m_context->fileLineNumbers[i], m_context->ppszEffectFile[i].c_str());
				return false;
			}

			if (Cmd == GenericSurfCommands[eCmds_Min]) { Params.minFilter = static_cast<SamplerFilter>(Type); }
			else if (Cmd == GenericSurfCommands[eCmds_Mag]) { Params.magFilter = static_cast<SamplerFilter>(Type); }
			else if (Cmd == GenericSurfCommands[eCmds_Mip]) { Params.mipFilter = static_cast<SamplerFilter>(Type); }
			else if (Cmd == GenericSurfCommands[eCmds_WrapR]) { Params.wrapR = static_cast<SamplerWrap>(Type); }
			else if (Cmd == GenericSurfCommands[eCmds_WrapS]) { Params.wrapS = static_cast<SamplerWrap>(Type); }
			else if (Cmd == GenericSurfCommands[eCmds_WrapT]) { Params.wrapT = static_cast<SamplerWrap>(Type); }
		}

		if (bKnown)
		{
			KnownCmds.push_back(Cmd);

			// Make sure nothing else exists on the line that hasn't been parsed.
			char8* pszRemaining = strtok(NULL, NEWLINE_TOKENS);
			if (pszRemaining)
			{
				pReturnError = strings::createFormatted("Unexpected keyword '%s' in [%s] on line %d: %s\n", pszRemaining, pCaller,
				                                        m_context->fileLineNumbers[i], m_context->ppszEffectFile[i].c_str());
				return false;
			}
		}
	}

	return true;
}
bool PfxReader::parseTexture(int32 nStartLine, int32 nEndLine, string& pReturnError)
{
	enum eCmd {	CmdsName,	CmdsPath,	CmdsView,	CmdsCamera, CmdsSize	};

	const StringHash TextureCmds[] =
	{
		StringHash("NAME"),				// eTextureCmds_Name
		StringHash("PATH"),				// eTextureCmds_Path
		StringHash("VIEW"),				// eTextureCmds_View
		StringHash("CAMERA"),			// eTextureCmds_Camera
	};
	PVR_STATIC_ASSERT(sizeof(TextureCmds) / sizeof(TextureCmds[0]) == CmdsSize, TextureCmds);

	PFXParserTexture TexDesc;
	TexDesc.minFilter = SamplerFilter::Default;
	TexDesc.magFilter = SamplerFilter::Default;
	TexDesc.mipFilter = SamplerFilter::MipDefault;
	TexDesc.wrapS = SamplerWrap::Default;
	TexDesc.wrapT = SamplerWrap::Default;
	TexDesc.wrapR = SamplerWrap::Default;
	TexDesc.width  = ViewportSize;
	TexDesc.height = ViewportSize;
	TexDesc.flags = PixelFormat::RGBA_8888.getPixelTypeId() | g_PfxTexColor;

	std::vector<StringHash> KnownCmds;
	if (!parseGenericSurface(nStartLine, nEndLine, TexDesc, KnownCmds, "TEXTURE", pReturnError))
	{
		return false;
	}

	string texName, filePath, viewName;
	for (int32 i = nStartLine + 1; i < nEndLine; i++)
	{
		// Skip blank lines
		if (m_context->ppszEffectFile[i].empty()) {	continue;	}

		char8* str = strtok(const_cast<char8*>(m_context->ppszEffectFile[i].c_str()), NEWLINE_TOKENS DELIM_TOKENS);
		if (!str)
		{
			pReturnError = strings::createFormatted("Missing arguments in [TEXTURE] on line %d: %s\n", m_context->fileLineNumbers[i],
			                                        m_context->ppszEffectFile[i].c_str());
			return false;
		}

		StringHash texCmd(str);
		// --- Texture name
		if (texCmd == TextureCmds[CmdsName])
		{
			char8* pszRemaining = strtok(NULL, NEWLINE_TOKENS DELIM_TOKENS);
			if (!pszRemaining)
			{
				pReturnError = strings::createFormatted("Missing NAME arguments in [TEXTURE] on line %d: %s\n", m_context->fileLineNumbers[i],
				                                        m_context->ppszEffectFile[i].c_str());
				return false;
			}

			texName = pszRemaining;
		}
		// --- Texture Path
		else if (texCmd == TextureCmds[CmdsPath])
		{
			char8* pszRemaining = strtok(NULL, NEWLINE_TOKENS);
			if (!pszRemaining)
			{
				pReturnError = strings::createFormatted("Missing PATH arguments in [TEXTURE] on line %d: %s\n", m_context->fileLineNumbers[i],
				                                        m_context->ppszEffectFile[i].c_str());
				return false;
			}

			if (!readStringToken(pszRemaining, filePath, pReturnError, i, "TEXTURE"))
			{
				return false;
			}
		}
		// --- View/Camera name
		else if (texCmd == TextureCmds[CmdsView] || texCmd == TextureCmds[CmdsCamera])
		{
			char8* pszRemaining = strtok(NULL, NEWLINE_TOKENS);		// String component. Get the rest of the line.
			if (!pszRemaining || strlen(pszRemaining) == 0)
			{
				pReturnError = strings::createFormatted("Missing VIEW argument in [TEXTURE] on line %d: %s\n", m_context->fileLineNumbers[i],
				                                        m_context->ppszEffectFile[i].c_str());
				return false;
			}

			if (!readStringToken(pszRemaining, viewName, pReturnError, i, "TEXTURE"))
			{
				return false;
			}
		}
		else if (std::find(KnownCmds.begin(), KnownCmds.end(), texCmd) != KnownCmds.end())
		{
			// Remove from 'unknown' list.
			for (uint32 uiIndex = 0; uiIndex < KnownCmds.size(); ++uiIndex)
			{
				if (KnownCmds[uiIndex] == texCmd)
				{
					KnownCmds.erase(KnownCmds.begin() + uiIndex);
					break;
				}
			}

			continue;		// This line has already been processed.
		}
		else
		{
			pReturnError = strings::createFormatted("Unknown keyword '%s' in [TEXTURE] on line %d: %s\n", str, m_context->fileLineNumbers[i],
			                                        m_context->ppszEffectFile[i].c_str());
			return false;
		}

		char8* pszRemaining = strtok(NULL, NEWLINE_TOKENS);
		if (pszRemaining)
		{
			pReturnError = strings::createFormatted("Unexpected keyword '%s' in [TEXTURE] on line %d: %s\n", pszRemaining,
			                                        m_context->fileLineNumbers[i], m_context->ppszEffectFile[i].c_str());
			return false;
		}
	}

	if (texName.empty())
	{
		pReturnError = strings::createFormatted("No NAME tag specified in [TEXTURE] on line %d\n",
		                                        m_context->fileLineNumbers[nStartLine]);
		return false;
	}
	if (!filePath.empty() && !viewName.empty())
	{
		pReturnError = strings::createFormatted("Both PATH and VIEW tags specified in [TEXTURE] on line %d\n",
		                                        m_context->fileLineNumbers[nStartLine]);
		return false;
	}
	if (filePath.empty() && viewName.empty())
	{
		pReturnError = strings::createFormatted("No PATH or VIEW tag specified in [TEXTURE] on line %d\n",
		                                        m_context->fileLineNumbers[nStartLine]);
		return false;
	}

	bool bRTT = (viewName.empty() ? false : true);
	if (bRTT)
	{
		filePath = texName;									// RTT doesn't have a physical file.
	}

	// Create a new texture and copy over the vals.
	PFXParserTexture* pTex = new PFXParserTexture();
	pTex->name.assign(texName);
	pTex->fileName.assign(filePath);
	pTex->renderToTexture	= bRTT;
	pTex->minFilter				= TexDesc.minFilter;
	pTex->magFilter				= TexDesc.magFilter;
	pTex->mipFilter				= TexDesc.mipFilter;
	pTex->wrapS			= TexDesc.wrapS;
	pTex->wrapT			= TexDesc.wrapT;
	pTex->wrapR			= TexDesc.wrapR;
	pTex->width			= TexDesc.width;
	pTex->height			= TexDesc.height;
	pTex->flags			= TexDesc.flags;
	m_textures.push_back(pTex);

	if (bRTT)
	{
		m_renderPasses.push_back(PfxRenderPass());
		uint32 uiPassIdx = (uint32)m_renderPasses.size() - 1;
		m_renderPasses[uiPassIdx].semanticName = texName;

		if (viewName == g_CurrentViewStr)
		{
			m_renderPasses[uiPassIdx].viewType = EffectPassView::Current;
		}
		else
		{
			m_renderPasses[uiPassIdx].viewType = EffectPassView::PodCamera;
			m_renderPasses[uiPassIdx].nodeName	 = viewName;
		}

		m_renderPasses[uiPassIdx].renderPassType = EffectPassType::Camera;			// textures are always 'camera' passes

		// Set render pass texture to the newly created texture.
		m_renderPasses[uiPassIdx].texture		 = pTex;
		m_renderPasses[uiPassIdx].formatFlags  = TexDesc.flags;
	}

	return true;
}

bool PfxReader::parseTarget(int32 nStartLine, int32 nEndLine, string& pReturnError)
{
	enum Cmd {CmdsName, CmdsSize};

	const StringHash TargetCommands[] =
	{
		StringHash("NAME"),				// eCmds_Name
	};
	PVR_STATIC_ASSERT(sizeof(TargetCommands) / sizeof(TargetCommands[0]) == CmdsSize, TargetCommands);

	string targetName;
	PFXParserTexture TexDesc;
	TexDesc.minFilter = SamplerFilter::Default;
	TexDesc.magFilter = SamplerFilter::Default;
	TexDesc.mipFilter = SamplerFilter::MipDefault;
	TexDesc.wrapS = SamplerWrap::Default;
	TexDesc.wrapT = SamplerWrap::Default;
	TexDesc.wrapR = SamplerWrap::Default;
	TexDesc.width  = ViewportSize;
	TexDesc.height = ViewportSize;
	TexDesc.flags = PixelFormat::RGBA_8888.getPixelTypeId() | g_PfxTexColor;

	std::vector<StringHash> KnownCmds;
	if (!parseGenericSurface(nStartLine, nEndLine, TexDesc, KnownCmds, "TARGET", pReturnError))
	{
		return false;
	}

	for (int32 i = nStartLine + 1; i < nEndLine; i++)
	{
		// Skip blank lines
		if (m_context->ppszEffectFile[i].empty()) {	continue;	}

		char8* str = strtok(const_cast<char8*>(m_context->ppszEffectFile[i].c_str()), NEWLINE_TOKENS DELIM_TOKENS);
		if (!str)
		{
			pReturnError = strings::createFormatted("Missing arguments in [TARGET] on line %d\n", m_context->fileLineNumbers[i]);
			return false;
		}

		StringHash texCmd(str);
		// --- Target name
		if (texCmd == TargetCommands[CmdsName])
		{
			char8* pszRemaining = strtok(NULL, NEWLINE_TOKENS DELIM_TOKENS);
			if (!pszRemaining)
			{
				pReturnError = strings::createFormatted("Missing NAME arguments in [TARGET] on line %d\n", m_context->fileLineNumbers[i]);
				return false;
			}

			targetName = pszRemaining;
		}
		else if (std::find(KnownCmds.begin(), KnownCmds.end(), texCmd) != KnownCmds.end())
		{
			// Remove from 'unknown' list.
			for (uint32 uiIndex = 0; uiIndex < KnownCmds.size(); ++uiIndex)
			{
				if (KnownCmds[uiIndex] == texCmd)
				{
					KnownCmds.erase(KnownCmds.begin() + uiIndex);
					break;
				}
			}

			continue;		// This line has already been processed.
		}
		else
		{
			pReturnError = strings::createFormatted("Unknown keyword '%s' in [TARGET] on line %d\n", str, m_context->fileLineNumbers[i]);
			return false;
		}

		char8* pszRemaining = strtok(NULL, NEWLINE_TOKENS);
		if (pszRemaining)
		{
			pReturnError = strings::createFormatted("Unexpected keyword '%s' in [TARGET] on line %d\n", pszRemaining,
			                                        m_context->fileLineNumbers[i]);
			return false;
		}
	}

	// Create a new texture and copy over the vals.
	PFXParserTexture* pTex = new PFXParserTexture();
	pTex->name.assign(targetName);
	pTex->fileName.assign(targetName);
	pTex->renderToTexture	= true;
	pTex->minFilter				= TexDesc.minFilter;
	pTex->magFilter				= TexDesc.magFilter;
	pTex->mipFilter				= TexDesc.mipFilter;
	pTex->wrapS			= TexDesc.wrapS;
	pTex->wrapT			= TexDesc.wrapT;
	pTex->wrapR			= TexDesc.wrapR;
	pTex->width			= TexDesc.width;
	pTex->height			= TexDesc.height;
	pTex->flags			= TexDesc.flags;
	m_textures.push_back(pTex);

	// Copy to render pass struct
	m_renderPasses.push_back(PfxRenderPass());
	uint32 uiPassIdx = (uint32)m_renderPasses.size() - 1;
	m_renderPasses[uiPassIdx].semanticName		= targetName;
	m_renderPasses[uiPassIdx].viewType = EffectPassView::None;
	m_renderPasses[uiPassIdx].renderPassType = EffectPassType::PostProcess;			// Targets are always post-process passes.
	m_renderPasses[uiPassIdx].texture			= pTex;
	m_renderPasses[uiPassIdx].formatFlags		= TexDesc.flags;

	return true;
}

bool PfxReader::parseTextures(int32 nStartLine, int32 nEndLine, string& pReturnError)
{
	string pszName, pszFile, pszKeyword, pszTemp;
	char8* pszRemaining(NULL);

	for (int32 i = nStartLine + 1; i < nEndLine; i++)
	{
		// Skip blank lines
		if (m_context->ppszEffectFile[i].empty()) {	continue;	}

		char8* str = strtok(const_cast<char8*>(m_context->ppszEffectFile[i].c_str()), " ");
		if (str != NULL)
		{
			// Set defaults
			SamplerFilter	uiMin(SamplerFilter::Default), uiMag(SamplerFilter::Default), uiMip(SamplerFilter::MipDefault);
			SamplerWrap uiWrapS(SamplerWrap::Default), uiWrapT(SamplerWrap::Default), uiWrapR(SamplerWrap::Default);
			uint32	flags = 0;

			uint32 width = PfxReader::ViewportSize;
			uint32 height = PfxReader::ViewportSize;

			// Reset variables
			pszName.clear();
			pszFile.clear();
			pszKeyword.clear();
			pszTemp.clear();
			pszRemaining = NULL;

			// Compare against all valid keywords
			if ((strcmp(str, "FILE") != 0) && (strcmp(str, "RENDER") != 0))
			{
				pReturnError = strings::createFormatted("Unknown keyword '%s' in [TEXTURES] on line %d\n", str, m_context->fileLineNumbers[i]);
				return false;
			}

#if 1
			if ((strcmp(str, "RENDER") == 0))
			{
				pReturnError = strings::createFormatted("RENDER tag no longer supported in [TEXTURES] block. Use new [TARGET] block instead\n");
				return false;
			}
#endif

			pszKeyword.assign(str);

			str = strtok(NULL, " ");
			if (str != NULL)
			{
				pszName.assign(str);
			}
			else
			{
				pReturnError = strings::createFormatted("Texture name missing in [TEXTURES] on line %d: %s\n", m_context->fileLineNumbers[i],
				                                        m_context->ppszEffectFile[i].c_str());
				return false;
			}

			/*
			The pszRemaining string is used to look for remaining flags.
			This has the advantage of allowing flags to be order independent
			and makes it easier to omit some flags, but still pick up others
			(the previous method made it difficult to retrieve filtering info
			if flags before it were missing)
			*/
			pszRemaining = strtok(NULL, "\n");

			if (pszRemaining == NULL)
			{
				pReturnError = strings::createFormatted("Incomplete definition in [TEXTURES] on line %d: %s\n", m_context->fileLineNumbers[i],
				                                        m_context->ppszEffectFile[i].c_str());
				return false;
			}
			else if (strcmp(pszKeyword.c_str(), "FILE") == 0)
			{

				pszTemp.assign(pszRemaining);
				str = strtok(const_cast<char8*>(pszTemp.c_str()), " ");

				if (str != NULL)
				{
					pszFile.assign(str);
				}
				else
				{
					pReturnError = strings::createFormatted("Texture name missing in [TEXTURES] on line %d: %s\n", m_context->fileLineNumbers[i],
					                                        m_context->ppszEffectFile[i].c_str());
					return false;
				}
			}

			if (strcmp(pszKeyword.c_str(), "FILE") == 0)
			{
				// --- Filter flags
				{
					SamplerFilter* pFlags[3] =
					{
						&uiMin,
						&uiMag,
						&uiMip,
					};

					if (!parseTextureFlags<SamplerFilter>(pszRemaining, pFlags, 3, Filters, (uint32)SamplerFilter::Size, pReturnError, i, m_context)) { return false; }
				}

				// --- Wrap flags
				{
					SamplerWrap* pFlags[3] =
					{
						&uiWrapS,
						&uiWrapT,
						&uiWrapR,
					};

					if (!parseTextureFlags<SamplerWrap>(pszRemaining, pFlags, 3, Wraps, (uint32)SamplerWrap::Size, pReturnError, i, m_context)) { return false; }
				}

				PFXParserTexture* pTex = new PFXParserTexture();
				pTex->name.assign(pszName);
				pTex->fileName.assign(pszFile);
				pTex->renderToTexture = false;
				pTex->minFilter = uiMin;
				pTex->magFilter = uiMag;
				pTex->mipFilter = uiMip;
				pTex->wrapS = uiWrapS;
				pTex->wrapT = uiWrapT;
				pTex->wrapR = uiWrapR;
				pTex->width = width;
				pTex->height = height;
				pTex->flags = flags;
				m_textures.push_back(pTex);
			}
			else
			{
				pReturnError = strings::createFormatted("Unknown keyword '%s' in [TEXTURES] on line %d\n", str, m_context->fileLineNumbers[i]);;
				return false;
			}
		}
		else
		{
			pReturnError = strings::createFormatted("Missing arguments in [TEXTURES] on line %d: %s\n", m_context->fileLineNumbers[i],
			                                        m_context->ppszEffectFile[i].c_str());
			return false;
		}
	}
	return true;
}

bool PfxReader::parseShader(int32 nStartLine, int32 nEndLine, string& pReturnError, PFXParserShader& shader,
                            const char8* const pszBlockName)
{
	enum Cmd
	{
		CmdsGLSLCode,
		CmdsName,
		CmdsFile,
		CmdsBinaryFile,

		CmdsSize
	};

	const StringHash ShaderCommands[] =
	{
		StringHash("[GLSL_CODE]"),
		StringHash("NAME"),
		StringHash("FILE"),
		StringHash("BINARYFILE"),
	};
	PVR_STATIC_ASSERT(sizeof(ShaderCommands) / sizeof(ShaderCommands[0]) == CmdsSize, ShaderCommands);

	bool glslcode = 0, glslfile = 0, bName = 0;

	shader.useFileName		= false;
	shader.firstLineNumPos	= 0;
	shader.lastLineNumPos  = 0;

	for (int32 i = nStartLine + 1; i < nEndLine; i++)
	{
		// Skip blank lines
		if (m_context->ppszEffectFile[i].empty()) {	continue;	}

		char8* str = strtok(const_cast<char8*>(m_context->ppszEffectFile[i].c_str()), " ");
		if (str != NULL)
		{
			StringHash Cmd(str);

			// Check for [GLSL_CODE] tags first and remove those lines from loop.
			if (Cmd == ShaderCommands[CmdsGLSLCode])
			{
				if (glslcode)
				{
					pReturnError = strings::createFormatted("[GLSL_CODE] redefined in [%s] on line %d\n", pszBlockName,
					                                        m_context->fileLineNumbers[i]);
					return false;
				}
				if (glslfile && !shader.glslBin.length())
				{
					pReturnError = strings::createFormatted("[GLSL_CODE] not allowed with FILE in [%s] on line %d\n", pszBlockName,
					                                        m_context->fileLineNumbers[i]);
					return false;
				}

				shader.firstLineNumPos = m_context->fileLineNumbers[i];

				// Skip the block-start
				i++;
				string GLSLCode;
				if (!strings::concatenateLinesUntil(
				      GLSLCode,
				      i,
				      m_context->ppszEffectFile,
				      m_context->nNumLines,
				      "[/GLSL_CODE]"))
				{
					return false;
				}

				shader.lastLineNumPos = m_context->fileLineNumbers[i];
				shader.glslCode = GLSLCode;
				shader.useFileName = false;
				glslcode = 1;
			}
			else if (Cmd == ShaderCommands[CmdsName])
			{
				if (bName)
				{
					pReturnError = strings::createFormatted("NAME redefined in [%s] on line %d\n", pszBlockName, m_context->fileLineNumbers[i]);
					return false;
				}

				str = strings::readEOLToken(NULL);

				if (str == NULL)
				{
					pReturnError = strings::createFormatted("NAME missing value in [%s] on line %d\n", pszBlockName, m_context->fileLineNumbers[i]);
					return false;
				}

				shader.name.assign(str);
				bName = true;
			}
			else if (Cmd == ShaderCommands[CmdsFile])
			{
				if (glslfile)
				{
					pReturnError = strings::createFormatted("FILE redefined in [%s] on line %d\n", pszBlockName, m_context->fileLineNumbers[i]);
					return false;
				}
				if (glslcode)
				{
					pReturnError = strings::createFormatted("FILE not allowed with [GLSL_CODE] in [%s] on line %d\n", pszBlockName,
					                                        m_context->fileLineNumbers[i]);
					return false;
				}

				str = strings::readEOLToken(NULL);

				if (str == NULL)
				{
					pReturnError = strings::createFormatted("FILE missing value in [%s] on line %d\n", pszBlockName, m_context->fileLineNumbers[i]);
					return false;
				}


				shader.glslFile = str;
				FileStream glslFile(str, "rb");

				if (!glslFile.open())
				{
					pReturnError = strings::createFormatted("Error loading file '%s' in [%s] on line %d\n", str, pszBlockName,
					                                        m_context->fileLineNumbers[i]);
					return false;
				}
				std::vector<char> glslCode;
				glslCode.resize(glslFile.getSize() + 1);
				size_t dataRead;
				if (!glslFile.read(glslFile.getSize(), 1, &glslCode[0], dataRead)) { return false; }
				shader.glslCode.assign(&glslCode[0]);
				shader.glslCode[glslFile.getSize()] = '\0';

				shader.firstLineNumPos = m_context->fileLineNumbers[i];		// Mark position where GLSL file is defined.

				shader.useFileName = true;
				glslfile = 1;
			}
			else if (Cmd == ShaderCommands[CmdsBinaryFile])
			{
				str = strings::readEOLToken(NULL);

				if (str == NULL)
				{
					pReturnError = strings::createFormatted("BINARYFILE missing value in [%s] on line %d\n", pszBlockName,
					                                        m_context->fileLineNumbers[i]);
					return false;
				}

				shader.glslBinFile = str;
				FileStream GLSLFile(str, "rb");

				if (!GLSLFile.open())
				{
					pReturnError = strings::createFormatted("Error loading file '%s' in [%s] on line %d\n", str, pszBlockName,
					                                        m_context->fileLineNumbers[i]);
					return false;
				}
				std::vector<char> shaderBin;
				shaderBin.resize((uint32)GLSLFile.getSize());
				size_t dataRead;
				if (!GLSLFile.read(shaderBin.size(), 1, &shaderBin[0], dataRead))
				{
					return false;
				}
				shader.glslBin.assign(&shaderBin[0]);
				shader.useFileName = true;
				glslfile = 1;
			}
			else
			{
				pReturnError = strings::createFormatted("Unknown keyword '%s' in [%s] on line %d\n", str, pszBlockName,
				                                        m_context->fileLineNumbers[i]);
				return false;
			}

			str = strtok(NULL, " ");
			if (str != NULL)
			{
				pReturnError = strings::createFormatted("Unexpected data in [%s] on line %d: '%s'\n", pszBlockName, m_context->fileLineNumbers[i],
				                                        str);
				return false;
			}
		}
		else
		{
			pReturnError = strings::createFormatted("Missing arguments in [%s] on line %d: %s\n", pszBlockName, m_context->fileLineNumbers[i],
			                                        m_context->ppszEffectFile[i].c_str());
			return false;
		}
	}

	if (!bName)
	{
		pReturnError = strings::createFormatted("NAME not found in [%s] on line %d.\n", pszBlockName,
		                                        m_context->fileLineNumbers[nStartLine]);
		return false;
	}

	if (!glslfile && !glslcode)
	{
		pReturnError = strings::createFormatted("No Shader File or Shader Code specified in [%s] on line %d\n", pszBlockName,
		                                        m_context->fileLineNumbers[nStartLine]);
		return false;
	}

	return true;
}

bool PfxReader::parseSemantic(EffectSemantic& semantic, int32 nStartLine, string& returnError)
{
	char8* str;

	semantic.variableName.assign("");
	semantic.sDefaultValue.type = SemanticDataType::None;

	str = strtok(NULL, " ");
	if (str == NULL)
	{
		returnError = strings::createFormatted("UNIFORM missing name in [EFFECT] on line %d\n", m_context->fileLineNumbers[nStartLine]);
		return false;
	}

	semantic.variableName = str;

	str = strtok(NULL, " ");
	if (str == NULL)
	{
		returnError = strings::createFormatted("UNIFORM missing value in [EFFECT] on line %d\n", m_context->fileLineNumbers[nStartLine]);
		return false;
	}

	/*
		If the final digits of the semantic are a number they are
		stripped off and used as the index, with the remainder
		used as the semantic.
	*/
	{
		//size_t idx, len;
		//len = strlen(str);

		//idx = len;
		//while (idx)
		//{
		//	--idx;
		//	if (strcspn(&str[idx], "0123456789") != 0)
		//	{
		//		break;
		//	}
		//}
		//if (idx == 0)
		//{
		//	returnError = strings::createFormatted("Semantic contains only numbers in [EFFECT] on line %d\n",
		//	                                       m_context->fileLineNumbers[nStartLine]);
		//	return false;
		//}

		//++idx;
		//int32 ordinal;
		//// Store the semantic index
		//if (len == idx)
		//{
		//	ordinal = 0;
		//}
		//else
		//{
		//	ordinal = atoi(&str[idx]);
		//}

		//// Chop off the index from the string containing the semantic
		//str[idx] = 0;
		//// Store a copy of the semantic name
		semantic.semantic.assign(str);
	}

	/*
		Optional default semantic value
	*/
	char8 pszString[2048];
	strcpy(pszString, "");
	str = strtok(NULL, " ");
	if (str != NULL)
	{
		// Get all ramainning arguments
		while (str != NULL)
		{
			strcat(pszString, str);
			strcat(pszString, " ");
			str = strtok(NULL, " ");
		}

		// default value
		int32 i;
		for (i = 0; i < (uint32)SemanticDataType::Count; i++)
		{
			const EffectSemanticDefaultDataTypeInfo& sDfltType =
			  EffectSemanticDefaultDataTypeInfo::getSemanticDefaultTypeInfo(SemanticDataType(i));
			if (strncmp(pszString, sDfltType.name, strlen(sDfltType.name)) == 0)
			{
				if (!getSemanticDataFromString(semantic.sDefaultValue, &pszString[strlen(sDfltType.name)], sDfltType.type, returnError))
				{
					returnError = strings::createFormatted(" on line %d.\n", m_context->fileLineNumbers[nStartLine]);
					return false;
				}
				semantic.sDefaultValue.type = sDfltType.type;
				break;
			}
		}

		// invalid data type
		if (i == (uint32)SemanticDataType::Count)
		{
			returnError = strings::createFormatted("'%s' unknown on line %d.\n", pszString, m_context->fileLineNumbers[nStartLine]);
			return false;
		}

	}

	return true;
}

bool PfxReader::parseEffect(PfxParserEffect& effect, int32 nStartLine, int32 nEndLine, string& returnError)
{
	enum Cmds
	{
		CmdsAnnotation,
		CmdsVertexShader,
		CmdsFragmentShader,
		CmdsTexture,
		CmdsUniform,
		CmdsAttribute,
		CmdsName,
		CmdsTarget,
		CmdsSize
	};

	const StringHash effectCommands[] =
	{
		StringHash("[ANNOTATION]"),
		StringHash("VERTEXSHADER"),
		StringHash("FRAGMENTSHADER"),
		StringHash("TEXTURE"),
		StringHash("UNIFORM"),
		StringHash("ATTRIBUTE"),
		StringHash("NAME"),
		StringHash("TARGET"),
	};
	PVR_STATIC_ASSERT(sizeof(effectCommands) / sizeof(effectCommands[0]) == CmdsSize, effectCommands);

	bool bName = false;
	bool bVertShader = false;
	bool bFragShader = false;

	for (int32 i = nStartLine + 1; i < nEndLine; i++)
	{
		// Skip blank lines
		if (m_context->ppszEffectFile[i].empty())	{	continue;	}

		char8* str = strtok(const_cast<char8*>(m_context->ppszEffectFile[i].c_str()), " ");

		if (str != NULL)
		{
			StringHash Cmd(str);

			if (Cmd == effectCommands[CmdsAnnotation])
			{
				if (!effect.annotation.empty())
				{
					returnError = strings::createFormatted("ANNOTATION redefined in [EFFECT] on line %d: \n", m_context->fileLineNumbers[i]);
					return false;
				}

				i++;		// Skip the block-start
				if (!strings::concatenateLinesUntil(
				      effect.annotation,
				      i,
				      m_context->ppszEffectFile,
				      m_context->nNumLines,
				      "[/ANNOTATION]"))
				{
					return false;
				}
			}
			else if (Cmd == effectCommands[CmdsVertexShader])
			{
				if (bVertShader)
				{
					returnError = strings::createFormatted("VERTEXSHADER redefined in [EFFECT] on line %d: \n", m_context->fileLineNumbers[i]);
					return false;
				}

				str = strings::readEOLToken(NULL);

				if (str == NULL)
				{
					returnError = strings::createFormatted("VERTEXSHADER missing value in [EFFECT] on line %d\n", m_context->fileLineNumbers[i]);
					return false;
				}
				effect.vertexShaderName.assign(str);
				bVertShader = true;
			}
			else if (Cmd == effectCommands[CmdsFragmentShader])
			{
				if (bFragShader)
				{
					returnError = strings::createFormatted("FRAGMENTSHADER redefined in [EFFECT] on line %d: \n", m_context->fileLineNumbers[i]);
					return false;
				}

				str = strings::readEOLToken(NULL);

				if (str == NULL)
				{
					returnError = strings::createFormatted("FRAGMENTSHADER missing value in [EFFECT] on line %d\n", m_context->fileLineNumbers[i]);
					return false;
				}
				effect.fragmentShaderName.assign(str);

				bFragShader = true;
			}
			else if (Cmd == effectCommands[CmdsTexture])
			{
				effect.textures.push_back(PFXParserEffectTexture());
				uint32 uiTexIdx = (uint32)effect.textures.size() - 1;
				// texture number
				str = strtok(NULL, " ");
				if (str != NULL)
				{
					effect.textures[uiTexIdx].number = atoi(str);
				}
				else
				{
					returnError = strings::createFormatted("TEXTURE missing value in [EFFECT] on line %d\n", m_context->fileLineNumbers[i]);
					return false;
				}

				// texture name
				str = strtok(NULL, " ");
				if (str != NULL)
				{
					effect.textures[uiTexIdx].name.assign(str);
				}
				else
				{
					returnError = strings::createFormatted("TEXTURE missing value in [EFFECT] on line %d\n", m_context->fileLineNumbers[i]);
					return false;
				}
			}
			else if (Cmd == effectCommands[CmdsUniform])
			{
				effect.uniforms.push_back(EffectSemantic());
				uint32 uiUniformIdx = (uint32)effect.uniforms.size() - 1;
				if (!parseSemantic(effect.uniforms[uiUniformIdx], i, returnError))
				{
					return false;
				}

			}
			else if (Cmd == effectCommands[CmdsAttribute])
			{
				effect.attributes.push_back(EffectSemantic());
				uint32 uiAttribIdx = (uint32)effect.attributes.size() - 1;
				if (!parseSemantic(effect.attributes[uiAttribIdx], i, returnError))
				{
					return false;
				}
			}
			else if (Cmd == effectCommands[CmdsName])
			{
				if (bName)
				{
					returnError = strings::createFormatted("NAME redefined in [EFFECT] on line %d\n", m_context->fileLineNumbers[nStartLine]);
					return false;
				}

				str = strtok(NULL, " ");
				if (str == NULL)
				{
					returnError = strings::createFormatted("NAME missing value in [EFFECT] on line %d\n", m_context->fileLineNumbers[nStartLine]);
					return false;
				}

				effect.name.assign(str);
				bName = true;
			}
			else if (Cmd == effectCommands[CmdsTarget])
			{
				effect.targets.push_back(EffectTargetPair());
				uint32 uiIndex = (uint32)effect.targets.size() - 1;

				// Target requires 2 components
				string* pVals[] = {	&effect.targets[uiIndex].first, &effect.targets[uiIndex].second	};

				for (uint32 uiVal = 0; uiVal < 2; ++uiVal)
				{
					str = strtok(NULL, " ");
					if (str == NULL)
					{
						returnError = strings::createFormatted("TARGET missing value(s) in [EFFECT] on line %d\n",
						                                       m_context->fileLineNumbers[nStartLine]);
						return false;
					}

					*(pVals[uiVal]) = str;
				}
			}
			else
			{
				returnError = strings::createFormatted("Unknown keyword '%s' in [EFFECT] on line %d\n", str, m_context->fileLineNumbers[i]);
				return false;
			}
		}
		else
		{
			returnError = strings::createFormatted("Missing arguments in [EFFECT] on line %d: %s\n", m_context->fileLineNumbers[i],
			                                       m_context->ppszEffectFile[i].c_str());
			return false;
		}
	}

	// Check that every TEXTURE has a matching UNIFORM
	for (uint32 uiTex = 0; uiTex < effect.textures.size(); ++uiTex)
	{
		uint32 uiTexUnit = effect.textures[uiTex].number;
		const StringHash& texName = effect.textures[uiTex].name;
		// Find UNIFORM associated with the TexUnit (e.g TEXTURE0).
		bool bFound = false;
		char buff[10];
		for (uint32 uiUniform = 0; uiUniform < effect.uniforms.size(); ++uiUniform)
		{

			const EffectSemantic& sem = effect.uniforms[uiUniform];
			if (strings::startsWith(sem.semantic.str(), "TEXTURE"))
			{
				sprintf(buff, "%d", uiTexUnit);
				if (strings::endsWith(sem.semantic.str(), buff))
				{
					bFound = true;
					break;
				}
			}
		}

		if (!bFound)
		{
			returnError = strings::createFormatted("TEXTURE %s missing matching UNIFORM in [EFFECT] on line %d\n", texName.c_str(),
			                                       m_context->fileLineNumbers[nStartLine]);
			return false;
		}
	}


	if (!bName)
	{
		returnError = strings::createFormatted("No 'NAME' found in [EFFECT] on line %d\n", m_context->fileLineNumbers[nStartLine]);
		return false;
	}
	if (!bVertShader)
	{
		returnError = strings::createFormatted("No 'VERTEXSHADER' defined in [EFFECT] starting on line %d: \n",
		                                       m_context->fileLineNumbers[nStartLine - 1]);
		return false;
	}
	if (!bFragShader)
	{
		returnError = strings::createFormatted("No 'FRAGMENTSHADER' defined in [EFFECT] starting on line %d: \n",
		                                       m_context->fileLineNumbers[nStartLine - 1]);
		return false;
	}

	return true;
}

bool PfxReader::determineRenderPassDependencies(string& errorOut)
{
	uint32	ui(0), uj(0), uk(0);

	if (m_renderPasses.size() == 0) {return true;}

	// --- Add all render pass nodes to the skip graph.
	for (ui = 0; ui < m_renderPasses.size(); ++ui)
	{
		PfxRenderPass& Pass = m_renderPasses[ui];
		bool bFound = false;

		// Search all EFFECT blocks for matching TARGET. This is for post-processes behavior.
		for (uint32 uiEffect = 0; uiEffect < m_effects.size(); ++uiEffect)
		{
			PfxParserEffect& Effect = m_effects[uiEffect];

			// Search all TARGETs in this effect
			for (uint32 uiTargets = 0; uiTargets < Effect.targets.size(); ++uiTargets)
			{
				const EffectTargetPair& Target = Effect.targets[uiTargets];
				if (Target.second == Pass.semanticName)
				{
					// Match. This EFFECT block matches the pass name.
					Pass.effect = &Effect;
					bFound = true;

					// This is now a post-process pass. Set relevant values.
					Pass.renderPassType = EffectPassType::PostProcess;
					m_postProcessNames.push_back(Pass.semanticName);

					// Check that the surface type and output match are relevant (i.e DEPTH != RGBA8888).
					if ((Target.first.find_first_of("DEPTH") != string::npos && !(Pass.formatFlags & g_PfxTexDepth))
					    || (Target.first.find_first_of("COLOR") != string::npos && !(Pass.formatFlags & g_PfxTexColor)))
					{
						errorOut = strings::createFormatted("Surface type mismatch in [EFFECT]. \"%s\" has different type than \"%s\"\n",
						                                    Target.second.c_str(), Pass.semanticName.c_str());
						return false;
					}

					break;
				}
			}

			if (bFound)	{	break;	}
		}

		// Add a pointer to the post process
		m_renderPassSkipGraph.addNode(&Pass);
	}


	// --- Loop through all created render passes in the skip graph and determine their dependencies
	for (ui = 0; ui < m_renderPassSkipGraph.getNumNodes(); ++ui)
	{
		//	Loop through all other nodes in the skip graph
		PfxRenderPass* pPass			= m_renderPassSkipGraph[ui];
		PfxRenderPass* pTestPass       = NULL;

		for (uj = 0; uj < m_renderPasses.size(); ++uj)
		{
			pTestPass = m_renderPassSkipGraph[uj];

			// No self compare
			if (pPass == pTestPass)	{	continue;}

			// No effect associated.
			if (!pPass->effect)	{	continue;}

			// Is the node a render pass I rely on?
			for (uk = 0; uk < pPass->effect->textures.size(); ++uk)
			{
				/*
					If the texture names match, add a new node
				*/
				if (pTestPass->texture->name == pPass->effect->textures[uk].name)
				{
					m_renderPassSkipGraph.addNodeDependency(pPass, pTestPass);
					break;
				}
			}
		}
	}

	return true;
}

uint32 PfxReader::findTextureIndex(const StringHash& TextureName, uint32 uiEffect) const
{
	for (uint32 uiIndex = 0; uiIndex < m_effects[uiEffect].textures.size(); ++uiIndex)
	{
		const PFXParserEffectTexture& Tex = m_effects[uiEffect].textures[uiIndex];
		if (Tex.name == TextureName) {	return uiIndex;	}
	}
	return 0xFFFFFFFF;
}

const PFXParserTexture* PfxReader::getTexture(uint32 uiIndex) const
{
	assertion(uiIndex < getNumberTextures());
	return m_textures[uiIndex];
}

int32 PfxReader::getEffectId(const StringHash& name) const
{
	if (name.getHash() == 0) {	return -1;	}

	for (uint32 uiIndex = 0; uiIndex < getNumberEffects(); ++uiIndex)
	{
		if (getParserEffect(uiIndex).name == name) {	return (int32)uiIndex;	}
	}

	return -1;
}

int32 PfxReader::findTextureByName(const StringHash& name) const
{
	if (name.getHash() == 0) {	return -1;	}

	for (uint32 uiIndex = 0; uiIndex < getNumberTextures(); ++uiIndex)
	{
		if (getTexture(uiIndex)->name == name)	{	return (int32)uiIndex;}
	}

	return -1;
}

bool PfxReader::readNextAsset(Effect& asset)
{
	if (!m_currentEffect)
	{
		uint32 dataSize = (uint32)m_assetStream->getSize();
		std::vector<char> data(dataSize + 1, '\0');// allocate enough space and null terminate.
		size_t dataRead;
		// read the data and parse
		if (!m_assetStream->read(dataSize, 1, &data[0], dataRead))
		{
			return false;
		}
		std::string errorStr;
		if (!parseFromMemory(&data[0], errorStr))
		{
			return false;
		}
	}
	return readEffect(asset, m_currentEffect++);
}

bool PfxReader::readEffect(Effect& asset, uint32 id) const
{
	const assets::PfxParserEffect& parserEffect = getParserEffect(id);
	// Create room for per-texture data
	const std::vector<PFXParserEffectTexture>& effectTextures = parserEffect.textures;
	uint32 numTextures = (uint32)effectTextures.size();
	m_textures.reserve(numTextures);
	asset.material.setEffectName(parserEffect.name);
	asset.fileName = m_fileName;
	// Initialize each Texture
	for (uint32 i = 0; i < numTextures; ++i)
	{
		int32 iTexIdx = findTextureByName(effectTextures[i].name);
		if (iTexIdx < 0)
		{
			Log(Log.Debug, "Effect '%s' requests non-existent texture: %s\n", parserEffect.name.c_str(),
			    effectTextures[i].name.c_str());
			return false;
		}
		asset.textures.push_back(assets::EffectTexture());
		EffectTexture& theTexture = asset.textures.back();

		const assets::PFXParserTexture& parserTex = *getTexture((uint32)asset.textures.size() - 1);
		theTexture.name = parserTex.name;
		theTexture.fileName = parserTex.fileName;
		theTexture.flags = 0;
		theTexture.unit = parserEffect.textures[i].number;
		theTexture.minFilter = parserTex.minFilter;
		theTexture.magFilter = parserTex.magFilter;
		theTexture.mipFilter = parserTex.mipFilter;
		theTexture.wrapR = parserTex.wrapR;
		theTexture.wrapS = parserTex.wrapS;
		theTexture.wrapT = parserTex.wrapT;
	}

	// find the vertex shader
	// find shaders requested
	for (uint32 vertShaderIdx = 0; vertShaderIdx < getNumberVertexShaders(); ++vertShaderIdx)
	{
		const PFXParserShader& vertShader = getVertexShader(vertShaderIdx);
		if (parserEffect.vertexShaderName == vertShader.name)
		{
			asset.vertexShader.name = vertShader.name;
			if (vertShader.useFileName)
			{
				asset.vertexShader.glslFile = vertShader.glslFile;
				asset.vertexShader.useFileName = true;
			}
			else
			{
				if (vertShader.glslCode.empty()) {	continue;  /* No code specified.*/  }

				// offset glsl code by nFirstLineNumber
				//vertShader.assign(VertexShader.firstLineNumPos + 1, '\n');
				asset.vertexShader.glslCode = vertShader.glslCode;
				asset.vertexShader.useFileName = false;
				//TBD THE FOLLOWING ORIGINAL CODE SEEMS LIKE A BUG
				//asset.vertexShader.glslCode.append(vertShader.glslCode);
			}
			break;
		}
	}
	for (uint32 fragShaderIdx = 0; fragShaderIdx < getNumberFragmentShaders(); ++fragShaderIdx)
	{
		const PFXParserShader& fragmentShader = getFragmentShader(fragShaderIdx);
		if (parserEffect.fragmentShaderName == fragmentShader.name)
		{
			asset.fragmentShader.name = fragmentShader.name;
			if (fragmentShader.useFileName)
			{
				asset.fragmentShader.glslFile = fragmentShader.glslFile;
				asset.fragmentShader.useFileName = true;
			}
			else
			{
				if (fragmentShader.glslCode.empty())
				{
					continue;  // No code specified.
				}

				// offset glsl code by nFirstLineNumber
				//fragShader.assign((fragmentShader.firstLineNumPos) + 1, '\n');
				asset.fragmentShader.glslCode = fragmentShader.glslCode;
				asset.fragmentShader.useFileName = false;
				//TBD THE FOLLOWING ORIGINAL CODE SEEMS LIKE A BUG
				//asset.fragmentShader.glslCode.append(fragmentShader.glslCode);
			}

			break;
		}
	}
	asset.uniforms = parserEffect.uniforms;
	asset.attributes = parserEffect.attributes;
	return true;
}


void pfxCreateStringCopy(char8** ppDst, const char8* pSrc)
{
	if (pSrc)
	{
		free(*ppDst);
		*ppDst = (char8*)malloc((strlen(pSrc) + 1) * sizeof(char8));
		strcpy(*ppDst, pSrc);
	}
}

const EffectSemanticDefaultDataTypeInfo& EffectSemanticDefaultDataTypeInfo::getSemanticDefaultTypeInfo(
  SemanticDataType semanticDfltType)
{
	assertion(semanticDfltType < SemanticDataType::Count, "Invalid Semantic Data Type");
	const static EffectSemanticDefaultDataTypeInfo g_semanticDefaultDataTypeInfo[] =
	{
		{ SemanticDataType::Mat2, "mat2", 4, EffectDefaultDataInternalType::Float },
		{ SemanticDataType::Mat3, "mat3", 9, EffectDefaultDataInternalType::Float },
		{ SemanticDataType::Mat4, "mat4", 16, EffectDefaultDataInternalType::Float },
		{ SemanticDataType::Vec2, "vec2", 2, EffectDefaultDataInternalType::Float },
		{ SemanticDataType::Vec3, "vec3", 3, EffectDefaultDataInternalType::Float },
		{ SemanticDataType::Vec4, "vec4", 4, EffectDefaultDataInternalType::Float },
		{ SemanticDataType::IVec2, "ivec2", 2, EffectDefaultDataInternalType::Integer },
		{ SemanticDataType::IVec3, "ivec3", 3, EffectDefaultDataInternalType::Integer },
		{ SemanticDataType::IVec4, "ivec4", 4, EffectDefaultDataInternalType::Integer },
		{ SemanticDataType::BVec2, "bvec2", 2, EffectDefaultDataInternalType::Boolean },
		{ SemanticDataType::BVec3, "bvec3", 3, EffectDefaultDataInternalType::Boolean },
		{ SemanticDataType::BVec4, "bvec4", 4, EffectDefaultDataInternalType::Boolean },
		{ SemanticDataType::Float, "float", 1, EffectDefaultDataInternalType::Float },
		{ SemanticDataType::Int1, "int32", 1, EffectDefaultDataInternalType::Integer },
		{ SemanticDataType::Bool1, "bool", 1, EffectDefaultDataInternalType::Boolean },
	};
	return g_semanticDefaultDataTypeInfo[(uint32)semanticDfltType];
}

}
}
//!\endcond
