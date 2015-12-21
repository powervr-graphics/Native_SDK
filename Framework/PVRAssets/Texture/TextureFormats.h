/*!*********************************************************************************************************************
\file         PVRAssets/Texture/TextureFormats.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Contains necessary preprocessor definition tokens for several formats across different APIS, to allow use of them without inclusion
              of their respective API headers. Especially used by PVR Assets texture loading code.
***********************************************************************************************************************/
#pragma once
///////////////OPEN GL(ES) STARTS HERE////////////

/* glTypes */
#define GL_BYTE								0x1400
#define GL_UNSIGNED_BYTE					0x1401
#define GL_SHORT							0x1402
#define GL_UNSIGNED_SHORT					0x1403
#define GL_INT								0x1404
#define GL_UNSIGNED_INT						0x1405
#define GL_FLOAT							0x1406
#define GL_HALF_FLOAT						0x140B
#define GL_FIXED							0x140C

/* glSizedTypes */
#define GL_UNSIGNED_SHORT_4_4_4_4         0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1         0x8034
#define GL_UNSIGNED_SHORT_5_6_5           0x8363
#define GL_UNSIGNED_BYTE_3_3_2            0x8032
#define GL_UNSIGNED_SHORT_4_4_4_4         0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1         0x8034
#define GL_UNSIGNED_INT_8_8_8_8           0x8035
#define GL_UNSIGNED_INT_10_10_10_2        0x8036
#define GL_UNSIGNED_BYTE_2_3_3_REV        0x8362
#define GL_UNSIGNED_SHORT_5_6_5           0x8363
#define GL_UNSIGNED_SHORT_5_6_5_REV       0x8364
#define GL_UNSIGNED_SHORT_4_4_4_4_REV     0x8365
#define GL_UNSIGNED_SHORT_1_5_5_5_REV     0x8366
#define GL_UNSIGNED_INT_8_8_8_8_REV       0x8367
#define GL_UNSIGNED_INT_2_10_10_10_REV    0x8368
#define GL_UNSIGNED_SHORT_8_8_APPLE       0x85BA
#define GL_UNSIGNED_SHORT_8_8_REV_APPLE   0x85BB
#define GL_UNSIGNED_INT_10F_11F_11F_REV   0x8C3B
#define GL_UNSIGNED_INT_5_9_9_9_REV		  0x8C3E

/* glFormats */
#define GL_RED                            0x1903
#define GL_RG                             0x8227
#define GL_ALPHA                          0x1906
#define GL_RGB                            0x1907
#define GL_RGBA                           0x1908
#define GL_BGR                            0x80E0
#define GL_BGRA                           0x80E1
#define GL_LUMINANCE                      0x1909
#define GL_LUMINANCE_ALPHA                0x190A
#define GL_RG_INTEGER                     0x8228
#define GL_RED_INTEGER                    0x8D94
#define GL_RGB_INTEGER                    0x8D98
#define GL_RGBA_INTEGER                   0x8D99
#define GL_RED_SNORM                      0x8F90
#define GL_RG_SNORM                       0x8F91
#define GL_RGB_SNORM                      0x8F92
#define GL_RGBA_SNORM                     0x8F93
#define GL_LUMINANCE_SNORM                0x9011
#define GL_LUMINANCE_ALPHA_SNORM          0x9012
#define GL_INTENSITY_SNORM                0x9013
#define GL_ALPHA_SNORM                    0x9010
#define GL_SRGB							  0x8C40
#define GL_SRGB_ALPHA					  0x8C42
#define GL_SLUMINANCE_ALPHA				  0x8C44
#define GL_SLUMINANCE                     0x8C46

/* glSizedInternalFormats*/
// Some weird format enum... not really used probably?
#define GL_R3_G3_B2                       0x2A10

// Tiny sized types.
#define GL_ALPHA4                         0x803B
#define GL_ALPHA8                         0x803C
#define GL_ALPHA12                        0x803D
#define GL_ALPHA16                        0x803E
#define GL_LUMINANCE4                     0x803F
#define GL_LUMINANCE8                     0x8040
#define GL_LUMINANCE12                    0x8041
#define GL_LUMINANCE16                    0x8042
#define GL_LUMINANCE4_ALPHA4              0x8043
#define GL_LUMINANCE6_ALPHA2              0x8044
#define GL_LUMINANCE8_ALPHA8              0x8045
#define GL_LUMINANCE12_ALPHA4             0x8046
#define GL_LUMINANCE12_ALPHA12            0x8047
#define GL_LUMINANCE16_ALPHA16            0x8048
#define GL_INTENSITY                      0x8049
#define GL_INTENSITY4                     0x804A
#define GL_INTENSITY8                     0x804B
#define GL_INTENSITY12                    0x804C
#define GL_INTENSITY16                    0x804D
#define GL_RGB2						      0x804E
#define GL_RGB4                           0x804F
#define GL_RGB5                           0x8050
#define GL_RGB8                           0x8051
#define GL_RGB10                          0x8052
#define GL_RGB12                          0x8053
#define GL_RGB16                          0x8054
#define GL_RGBA2                          0x8055
#define GL_RGBA4                          0x8056
#define GL_RGB5_A1                        0x8057
#define GL_RGBA8                          0x8058
#define GL_RGB10_A2                       0x8059
#define GL_RGBA12                         0x805A
#define GL_RGBA16                         0x805B

// Regular sized r/rg/rgb/rgba
#define GL_R8                             0x8229
#define GL_R16                            0x822A
#define GL_RG8                            0x822B
#define GL_RG16                           0x822C
#define GL_R16F                           0x822D
#define GL_R32F                           0x822E
#define GL_RG16F                          0x822F
#define GL_RG32F                          0x8230
#define GL_R8I                            0x8231
#define GL_R8UI                           0x8232
#define GL_R16I                           0x8233
#define GL_R16UI                          0x8234
#define GL_R32I                           0x8235
#define GL_R32UI                          0x8236
#define GL_RG8I                           0x8237
#define GL_RG8UI                          0x8238
#define GL_RG16I                          0x8239
#define GL_RG16UI                         0x823A
#define GL_RG32I                          0x823B
#define GL_RG32UI                         0x823C
#define GL_RGBA32F                        0x8814
#define GL_RGB32F                         0x8815
#define GL_RGBA16F                        0x881A
#define GL_RGB16F                         0x881B
#define GL_RGBA32UI                       0x8D70
#define GL_RGB32UI                        0x8D71
#define GL_RGBA16UI                       0x8D76
#define GL_RGB16UI                        0x8D77
#define GL_RGBA8UI                        0x8D7C
#define GL_RGB8UI                         0x8D7D
#define GL_RGBA32I                        0x8D82
#define GL_RGB32I                         0x8D83
#define GL_RGBA16I                        0x8D88
#define GL_RGB16I                         0x8D89
#define GL_RGBA8I                         0x8D8E
#define GL_RGB8I                          0x8D8F

// Slightly odd R/RG/RGB/RGBA formats
#define GL_R11F_G11F_B10F                 0x8C3A
#define GL_RGB9_E5						  0x8C3D
#define GL_RGB565                         0x8D62
#define GL_RGB10_A2UI                     0x906F

// Floating point l/a/i types
#define GL_ALPHA32F_ARB                   0x8816
#define GL_INTENSITY32F_ARB               0x8817
#define GL_LUMINANCE32F_ARB               0x8818
#define GL_LUMINANCE_ALPHA32F_ARB         0x8819
#define GL_ALPHA16F_ARB                   0x881C
#define GL_INTENSITY16F_ARB               0x881D
#define GL_LUMINANCE16F_ARB               0x881E
#define GL_LUMINANCE_ALPHA16F_ARB         0x881F

// An Apple extension
#define GL_RGB_422_APPLE                  0x8A1F

// SRGB
#define GL_SRGB8                          0x8C41
#define GL_SRGB8_ALPHA8                   0x8C43
#define GL_SLUMINANCE8_ALPHA8             0x8C45
#define GL_SLUMINANCE8					  0x8C47

// Signed normalised types.
#define GL_R8_SNORM                       0x8F94
#define GL_RG8_SNORM                      0x8F95
#define GL_RGB8_SNORM                     0x8F96
#define GL_RGBA8_SNORM                    0x8F97
#define GL_R16_SNORM                      0x8F98
#define GL_RG16_SNORM                     0x8F99
#define GL_RGB16_SNORM                    0x8F9A
#define GL_RGBA16_SNORM                   0x8F9B
#define GL_ALPHA8_SNORM                   0x9014
#define GL_LUMINANCE8_SNORM               0x9015
#define GL_LUMINANCE8_ALPHA8_SNORM        0x9016
#define GL_INTENSITY8_SNORM               0x9017
#define GL_ALPHA16_SNORM                  0x9018
#define GL_LUMINANCE16_SNORM              0x9019
#define GL_LUMINANCE16_ALPHA16_SNORM      0x901A
#define GL_INTENSITY16_SNORM              0x901B

/* glCompressedFormats*/

//ETC1
#define GL_ETC1_RGB8_OES                                        0x8D64

//ETC2
#define GL_COMPRESSED_R11_EAC									0x9270
#define GL_COMPRESSED_SIGNED_R11_EAC							0x9271
#define GL_COMPRESSED_RG11_EAC									0x9272
#define GL_COMPRESSED_SIGNED_RG11_EAC							0x9273
#define GL_COMPRESSED_RGB8_ETC2									0x9274
#define GL_COMPRESSED_SRGB8_ETC2								0x9275
#define GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2				0x9276
#define GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2			0x9277
#define GL_COMPRESSED_RGBA8_ETC2_EAC							0x9278
#define GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC						0x9279

//PVRTC1
#define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG                      0x8C00
#define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG                      0x8C01
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG                     0x8C02
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG                     0x8C03

//PVRTC2
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG						0x9137
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG						0x9138

//DXTC
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT							0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT						0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE						0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE						0x83F3

//AMD Formats
#define GL_3DC_X_AMD                                            0x87F9
#define GL_3DC_XY_AMD                                           0x87FA
#define GL_ATC_RGB_AMD                                          0x8C92
#define GL_ATC_RGBA_EXPLICIT_ALPHA_AMD                          0x8C93
#define GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD                      0x87EE

//Compressed "glFormat" variables... originally designed for glCompressedTexImage2D??
#define GL_COMPRESSED_ALPHA										0x84E9
#define GL_COMPRESSED_LUMINANCE									0x84EA
#define GL_COMPRESSED_LUMINANCE_ALPHA							0x84EB
#define GL_COMPRESSED_INTENSITY									0x84EC
#define GL_COMPRESSED_RGB										0x84ED
#define GL_COMPRESSED_RGBA										0x84EE
#define GL_COMPRESSED_SRGB										0x8C48
#define GL_COMPRESSED_SRGB_ALPHA								0x8c49
#define GL_COMPRESSED_SLUMINANCE								0x8C4A
#define GL_COMPRESSED_SLUMINANCE_ALPHA							0x8C4B

//Palletted formats
#define GL_PALETTE4_RGB8_OES                                    0x8B90
#define GL_PALETTE4_RGBA8_OES                                   0x8B91
#define GL_PALETTE4_R5_G6_B5_OES                                0x8B92
#define GL_PALETTE4_RGBA4_OES                                   0x8B93
#define GL_PALETTE4_RGB5_A1_OES                                 0x8B94
#define GL_PALETTE8_RGB8_OES                                    0x8B95
#define GL_PALETTE8_RGBA8_OES                                   0x8B96
#define GL_PALETTE8_R5_G6_B5_OES                                0x8B97
#define GL_PALETTE8_RGBA4_OES                                   0x8B98
#define GL_PALETTE8_RGB5_A1_OES                                 0x8B99
