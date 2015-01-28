/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2001 Ben Martin

    libferris is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libferris is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libferris.  If not, see <http://www.gnu.org/licenses/>.

    For more details see the COPYING file in the root directory of this
    distribution.

    $Id: StatfsUtilities.cpp,v 1.2 2010/09/24 21:31:00 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <StatfsUtilities.hh>
#include <SignalStreams.hh>

#define SYSV_MAGIC_BASE         0x012FF7B3
FERRISEXP_DLLLOCAL enum {
    FSTYPE_NONE = 0,
    FSTYPE_XENIX,
    FSTYPE_SYSV4,
    FSTYPE_SYSV2,
    FSTYPE_COH,
    FSTYPE_V7,
    FSTYPE_AFS,
    FSTYPE_END,
};

#define ADFS_SUPER_MAGIC	 0xadf5
#define AFFS_SUPER_MAGIC 0xadff
#define CODA_SUPER_MAGIC	0x73757245
#define COH_SUPER_MAGIC		(SYSV_MAGIC_BASE+FSTYPE_COH)
#define DEVFS_SUPER_MAGIC                0x1373
#define EFS_SUPER_MAGIC	0x414A53
#define EXT2_SUPER_MAGIC        0xEF53
#define HPFS_SUPER_MAGIC 0xf995e849
#define ISOFS_SUPER_MAGIC 0x9660
#define JFFS2_SUPER_MAGIC 0x72b6
#define MINIX_SUPER_MAGIC       0x137F          /* original minix fs */
#define MINIX2_SUPER_MAGIC      0x2468          /* minix V2 fs */
#define MINIX2_SUPER_MAGIC2     0x2478          /* minix V2 fs, 30 char names */
#define MSDOS_SUPER_MAGIC 0x4d44 /* MD */
#define NCP_SUPER_MAGIC  0x564c
#define NFS_SUPER_MAGIC			0x6969
#define OPENPROM_SUPER_MAGIC 0x9fa1
#define PROC_SUPER_MAGIC 0x9fa0
#define QNX4_SUPER_MAGIC	0x002f	/* qnx4 fs detection */
#define REISERFS_SUPER_MAGIC 0x52654973
#define SMB_SUPER_MAGIC               0x517B
#define SYSV4_SUPER_MAGIC	(SYSV_MAGIC_BASE+FSTYPE_SYSV4)
#define SYSV2_SUPER_MAGIC	(SYSV_MAGIC_BASE+FSTYPE_SYSV2)
#define UFS_MAGIC             0x00011954
#define USBDEVICE_SUPER_MAGIC 0x9fa2
#define XFS_SUPER_MAGIC 0x58465342
#define XENIX_SUPER_MAGIC	(SYSV_MAGIC_BASE+FSTYPE_XENIX)

namespace Ferris
{
    namespace Util
    {
        std::string getFileSystemTypeString( const struct statfs& s )
        {
            fh_stringstream ss;
            
            switch( s.f_type )
            {
            case AFFS_SUPER_MAGIC:       ss << "affs";        break;
            case EXT2_SUPER_MAGIC:       ss << "ext2";        break;
            case HPFS_SUPER_MAGIC:       ss << "hpfs";        break;
            case ISOFS_SUPER_MAGIC:      ss << "iso";         break;
            case MINIX_SUPER_MAGIC:
            case MINIX2_SUPER_MAGIC:
            case MINIX2_SUPER_MAGIC2:    ss << "minix";       break;
            case MSDOS_SUPER_MAGIC:      ss << "msdos";       break;
            case NCP_SUPER_MAGIC:        ss << "ncp";         break;
            case NFS_SUPER_MAGIC:        ss << "nfs";         break;
            case PROC_SUPER_MAGIC:       ss << "proc";        break;
            case SMB_SUPER_MAGIC:        ss << "smb";         break;
            case XENIX_SUPER_MAGIC:
            case SYSV4_SUPER_MAGIC:
            case SYSV2_SUPER_MAGIC:
            case COH_SUPER_MAGIC:        ss << "sysv";        break;
            case UFS_MAGIC:              ss << "ufs";         break;
            case XFS_SUPER_MAGIC:        ss << "xfs";         break;
            default: ss << "unknown";
            }
    
            return tostr(ss);
        }
    }
};

