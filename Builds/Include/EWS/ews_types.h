/*************************************************************************/ /*!
@File
@Title          Example windowing system constants and datastructures.
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#if !defined(EWS_TYPES_H)
#define EWS_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* EWS API version number. This should be incremented every time a
   backwards-incompatible change is made to the API. */
#define EWS_API_VERSION 10

/* Client flags */
#define EWS_CLIENT_PRIVILEGED (1<<0) /* can read restricted properties */
#define EWS_CLIENT_WM         (1<<1) /* receives notifications of other
										clients creating windows */
#define EWS_CLIENT_COMPOSITOR (1<<2) /* client is a compositor */

/* Write flags */
#define EWS_WRITE_OVERWRITE (1<<0) /* WRITE_PROPERTY overwrites rather than inserts */
#define EWS_WRITE_APPEND    (1<<1) /* Data is appended to the end of the
									  property value, regardless of the
									  specified position */

/* Namespaces */
#define EWS_NAMESPACE_PUBLIC     0
#define EWS_NAMESPACE_RESTRICTED 1

typedef enum
{
	EWS_FALSE = 0,
	EWS_TRUE = 1,
} EWS_BOOL;

/* Errors */
/* When adding new error messages here, make sure it tallies with
   apszErrorStrings[] inside <ddk>/ews/common/errstring.h */
typedef enum
{
	EWS_ERROR_BAD_REQUEST   = 1,  /* A request opcode was unknown */
	EWS_ERROR_BAD_WINDOW    = 2,  /* A window named in the request did not exist */
	EWS_ERROR_BAD_COORD     = 3,  /* A coordinate given was out of range */
	EWS_ERROR_BAD_SIZE      = 4,  /* A size given was out of range */
	EWS_ERROR_BAD_FLAGS     = 5,  /* An invalid set of flags was specified */
	EWS_ERROR_BAD_DISPLAY   = 6,  /* A display given in the request did not exist */
	EWS_ERROR_BAD_ATOM      = 7,  /* An atom given was not defined or was EWS_NO_ATOM */
	EWS_ERROR_BAD_ACCESS    = 8,  /* Access was denied */
	EWS_ERROR_BAD_RANGE     = 9,  /* A property position was out of range */
	EWS_ERROR_BAD_PROPERTY  = 10, /* A property named in GET_PROPERTY was unset */
	EWS_ERROR_BAD_VERSION   = 11, /* The client's version is incompatible with the server */
	EWS_ERROR_BAD_AUTH      = 12, /* The authentication data is incorrect */
	EWS_ERROR_BAD_ALLOC     = 13, /* Server ran out of memory */
	EWS_ERROR_BAD_OPERATION = 14, /* The requested operation could not be performed */
	EWS_ERROR_BAD_NAMESPACE = 15, /* A namespace given in the request did not exist */
	EWS_ERROR_LOCK_UNAVAILABLE = 16, /* No buffers could be locked */
	EWS_ERROR_NUM_ERRORS
} EWS_ERROR;


/* The types of windowing system resources from the client's point of view.
   EWS_WINDOW identifies a window to the server, which holds the real
   information about the window. */
typedef unsigned long EWS_ATOM;
typedef unsigned int EWS_CLIENT;
typedef unsigned int EWS_DISPLAY;
typedef unsigned int EWS_WINDOW;
typedef unsigned int EWS_PIXMAP;
typedef unsigned int EWS_ZORDER;

/* Special values for these types */
#define EWS_NO_ATOM         ((EWS_ATOM)0)
#define EWS_NO_CLIENT       ((EWS_CLIENT)0)
#define EWS_NO_DISPLAY      ((EWS_DISPLAY)0xFFFF)
#define EWS_DEFAULT_DISPLAY ((unsigned int)0)
#define EWS_NO_WINDOW       ((EWS_WINDOW)0)

#define EWS_ATOM_MAX    ((EWS_ATOM)0xFFFFFFFF)
#define EWS_CLIENT_MAX  ((EWS_CLIENT)0xFFFF)
#define EWS_DISPLAY_MAX ((EWS_DISPLAY)0xFFFF)
#define EWS_WINDOW_MAX  ((EWS_WINDOW)0xFFFF)
#define EWS_ZORDER_MAX  ((EWS_ZORDER)0xFFFF)

#define EWS_COORDELEMENT_MAX 32767
#define EWS_SIZEELEMENT_MAX  65535
#define EWS_NAMESPACE_MAX    255
#define EWS_BUFFERINDEX_MAX  255
#define EWS_FLAGS_MAX        255

typedef enum
{
	EWS_COLORSPACE_BT601_CONFORMANT_RANGE = 1,
	EWS_COLORSPACE_BT601_FULL_RANGE = 2,
	EWS_COLORSPACE_BT709_CONFORMANT_RANGE = 3,
	EWS_COLORSPACE_BT709_FULL_RANGE = 4,
}EWS_COLORSPACE;

typedef enum
{
	EWS_CHROMA_INTERP_ZERO = 1,
	EWS_CHROMA_INTERP_QUARTER = 2,
	EWS_CHROMA_INTERP_HALF = 3,
	EWS_CHROMA_INTERP_THREEQUARTERS = 4,
}EWS_CHROMA_INTERP;

typedef enum
{
	EWS_MEMLAYOUT_DEFAULT = 0,
	EWS_MEMLAYOUT_STRIDE = 1,
	EWS_MEMLAYOUT_PAGETILE = 2,
}EWS_MEMLAYOUT;

typedef enum
{
	EWS_PIXMAP_ATTRIB_NONE = 0,
	EWS_PIXMAP_ATTRIB_STRIDE = 1,
	EWS_PIXMAP_ATTRIB_YUV_COLORSPACE = 2,
	EWS_PIXMAP_ATTRIB_YUV_CHROMA_U_INTERP = 3,
	EWS_PIXMAP_ATTRIB_YUV_CHROMA_V_INTERP = 4,
	EWS_PIXMAP_ATTRIB_MEMLAYOUT = 5,

}EWS_PIXMAP_ATTRIB;

/* Pixel formats */
typedef enum
{
	EWS_PIXEL_FORMAT_ARGB_8888 = 1,
	EWS_PIXEL_FORMAT_RGB_565   = 2,
	EWS_PIXEL_FORMAT_ABGR_8888 = 3,
	EWS_PIXEL_FORMAT_ARGB_1555 = 4,
	EWS_PIXEL_FORMAT_ARGB_4444 = 5,
	EWS_PIXEL_FORMAT_XRGB_8888 = 6,
	EWS_PIXEL_FORMAT_XBGR_8888 = 7,
	EWS_PIXEL_FORMAT_YUYV      = 8,
	EWS_PIXEL_FORMAT_NV12      = 9,
	EWS_PIXEL_FORMAT_YV12      = 10,
	EWS_PIXEL_FORMAT_ARGB_8888_SRGB = 11,
	EWS_PIXEL_FORMAT_R_8       = 12,
	EWS_PIXEL_FORMAT_RG_88     = 13,
	EWS_PIXEL_FORMAT_NV21_MACROBLOCK = 14
} EWS_PIXELFORMAT;

#define EWS_PIXEL_FORMAT_MAX EWS_PIXEL_FORMAT_NV21_MACROBLOCK

/* Clockwise rotations */
typedef enum
{
	EWS_ROTATE_0   = 0,
	EWS_ROTATE_90  = 1,
	EWS_ROTATE_180 = 2,
	EWS_ROTATE_270 = 3,
} EWS_ROTATION;

/* Screen coordinates (positions and dimensions of windows) */
typedef struct EWS_COORD_TAG
{
	int iX;
	int iY;
} EWS_COORD;

typedef struct EWS_SIZE_TAG
{
	unsigned int uiWidth;
	unsigned int uiHeight;
} EWS_SIZE;

/* Sizes of protocol fields that the types above are packed into */
#define EWS_PIXELFORMAT_PACKEDSIZE 1
#define EWS_ROTATION_PACKEDSIZE    1
#define EWS_WINDOW_PACKEDSIZE      2
#define EWS_DISPLAY_PACKEDSIZE     2
#define EWS_CLIENT_PACKEDSIZE      2
#define EWS_ATOM_PACKEDSIZE        4
#define EWS_COORD_PACKEDSIZE       4
#define EWS_SIZE_PACKEDSIZE        4
#define EWS_MEMLAYOUT_PACKEDSIZE   1

/* Events */
typedef enum
{
	EWS_EVENT_CREATE_NOTIFY    = 1,
	EWS_EVENT_DESTROY_NOTIFY   = 2,
	EWS_EVENT_PROPERTY_NOTIFY  = 3,
	EWS_EVENT_RPROPERTY_NOTIFY = 4,
	EWS_EVENT_KEYPRESS         = 5,
} EWS_EVENT_TYPE;

typedef struct EWS_EVENT_TAG
{
	EWS_CLIENT sClient;
	EWS_WINDOW sWindow;
	EWS_EVENT_TYPE eType;
	union
	{
		/* These are packed into a 32-bit field. If any new union fields are
		   added here, update EWSProtoEvent and EWSProtoUnpackEvent in
		   protocol.c to handle them */
		EWS_ATOM sPropertyName;
		unsigned long uiDummy;
		unsigned long uiKeyCode;
	};
} EWS_EVENT;

typedef void (*EWS_ERROR_HANDLER)(int);
typedef void (*EWS_EVENT_HANDLER)(EWS_EVENT);

typedef struct EWS_SURFACE_INFO_TAG
{
	unsigned int uiNumBuffers;
	void *pvExportData;
} EWS_SURFACE_INFO;

/* The keycodes from an associated EWS_EVENT_KEYPRESS event
   These are the minumun to get the SDK demo working.
   The values are taken from the linux input system */
#define EWS_KEY_ESC   1
#define EWS_KEY_Q     16
#define EWS_KEY_S     31
#define EWS_KEY_ENTER 28
#define EWS_KEY_SPACE 57
#define EWS_KEY_UP    103
#define EWS_KEY_DOWN  108
#define EWS_KEY_LEFT  105
#define EWS_KEY_RIGHT 106

#ifdef __cplusplus
}
#endif

#endif /* EWS_TYPES_H */

