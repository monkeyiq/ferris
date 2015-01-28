; /******************************************************************************
; *******************************************************************************
; *******************************************************************************

;     libferris
;     Copyright (C) 2005 Ben Martin

;     This program is free software; you can redistribute it and/or modify
;     it under the terms of the GNU General Public License as published by
;     the Free Software Foundation; either version 2 of the License, or
;     (at your option) any later version.

;     This program is distributed in the hope that it will be useful,
;     but WITHOUT ANY WARRANTY; without even the implied warranty of
;     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;     GNU General Public License for more details.

;     You should have received a copy of the GNU General Public License
;     along with this program; if not, write to the Free Software
;     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

;     For more details see the COPYING file in the root directory of this
;     distribution.

;     $Id: libferris-emacs.el,v 1.1 2005/12/20 11:42:17 ben Exp $

; *******************************************************************************
; *******************************************************************************
; ******************************************************************************/


(defun libferris-list-buffers ()
(save-excursion
  (save-restriction
  (setq oldbuf (buffer-name))
  (switch-to-buffer nil)
  (setq oldbuf2 (buffer-name))
(progn
  (setq ferris-temp-path    (expand-file-name "~/.ferris/tmp/libferris-xemacs-list"))
  (setq ferris-temp-buffer  "libferris-xemacs-list")
  (switch-to-buffer (list-buffers-noselect))
  (copy-to-buffer ferris-temp-buffer (point-min) (point-max))
  (switch-to-buffer ferris-temp-buffer)
  (goto-char (point-min))
  (write-file ferris-temp-path)
  (kill-buffer ferris-temp-buffer)

)
(switch-to-buffer oldbuf2)
(switch-to-buffer oldbuf)
)))

(defun libferris-export-buffer (buffer-name-to-export)
(save-excursion
  (save-restriction
  (setq oldbuf (buffer-name))
  (switch-to-buffer nil)
  (setq oldbuf2 (buffer-name))
(progn
  (setq ferris-temp-path    (expand-file-name "~/.ferris/tmp/libferris-xemacs-export"))
  (setq ferris-temp-buffer  "libferris-xemacs-export")
  (switch-to-buffer buffer-name-to-export)
  (copy-to-buffer ferris-temp-buffer (point-min) (point-max))
  (switch-to-buffer ferris-temp-buffer)
  (goto-char (point-min))
  (write-file ferris-temp-path)
  (kill-buffer ferris-temp-buffer)
)
(switch-to-buffer oldbuf2)
(switch-to-buffer oldbuf)
)))

(defun libferris-import-buffer (buffer-name-to-import url-to-import)
(save-excursion
  (save-restriction
  (setq oldbuf (buffer-name))
  (switch-to-buffer nil)
  (setq oldbuf2 (buffer-name))
(progn
  (setq ferris-temp-path    (expand-file-name "~/.ferris/tmp/libferris-xemacs-import"))
  (setq ferris-temp-buffer  "libferris-xemacs-import")
  (find-file ferris-temp-path)
  (find-file url-to-import)
  (switch-to-buffer ferris-temp-buffer)
  (copy-to-buffer buffer-name-to-import (point-min) (point-max))
  (kill-buffer ferris-temp-buffer)
)
(switch-to-buffer oldbuf2)
(switch-to-buffer oldbuf)
(switch-to-buffer buffer-name-to-import)
)))

