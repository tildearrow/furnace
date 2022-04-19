/*                   */
/* FDS souce plug-in */
/*                   */
typedef	unsigned char	BYTE;
typedef	unsigned short	WORD;
typedef	signed int	INT;
typedef	void*		HFDS;

/* Function pointer prototypes */
/* Create */
typedef	HFDS (__cdecl *FDSCREATE )();
/* Close */
typedef	void (__cdecl *FDSCLOSE  )( HFDS );

/* Reset */
typedef	void (__cdecl *FDSRESET  )( HFDS, INT );
/* Setup */
typedef	void (__cdecl *FDSSETUP  )( HFDS, INT );
/* Write */
typedef	void (__cdecl *FDSWRITE  )( HFDS, WORD, BYTE );
/* Read */
typedef	BYTE (__cdecl *FDSREAD   )( HFDS, WORD );

/* Get PCM data */
typedef	INT  (__cdecl *FDSPROCESS)( HFDS );
/* Get FDS frequency */
typedef	INT  (__cdecl *FDSGETFREQ)( HFDS );

/* Write */
typedef	void (__cdecl *FDSWRITESYNC)( HFDS, WORD, BYTE );
/* Read */
typedef	BYTE (__cdecl *FDSREADSYNC)( HFDS, WORD );
/* Sync */
typedef	void (__cdecl *FDSSYNC   )( HFDS, INT );

