;; Setting Up The Presentation

(load-theme 'tango-dark) 
(tool-bar-mode -1)
(menu-bar-mode -1)
(scroll-bar-mode -1)
(setq display-time-day-and-date t
      display-time-24hr-format t)
(display-time)
(visual-line-mode 1)

(setq scroll-conservatively 10000)
(split-window-right)
(setq shift-select-mode nil)
(display-time-mode 1)
(delete-selection-mode 1)
(global-auto-revert-mode t)

(show-paren-mode 1)
(electric-pair-mode 1)
(setq show-paren-delay 0)

(set-default-font "DejaVu Sans Mono")
(set-face-attribute 'default nil :height 100)
(setq scroll-error-top-bottom t)
(setq mark-ring-max 2)

;; CamelCase Stuff

(global-subword-mode 1)

;; OrgMode

(require 'org)
(setq org-log-done 'time)
(setq org-indent-mode t)

;; Imenu
(setq imenu-auto-rescan t)
(setq imenu-auto-rescan-maxout 500000)
(setq make-backup-files nil)
(setq version-control nil)
(setq imenu-sort-function t)

;; hide-show
(add-hook 'prog-mode-hook 'hs-minor-mode)

;; Handle super-tabbify (TAB completes, shift-TAB actually tabs)
(setq dabbrev-case-replace t)
(setq dabbrev-case-fold-search t)
(setq dabbrev-upcase-means-case-search t)

;; Abbrevation expansion
(abbrev-mode 1)

;; Stupid Ascii tab behaviour Know C-i is H-i
(define-key input-decode-map (kbd "C-i") (kbd "H-i"))
;;(setq local-function-key-map (delq '(kp-tab . [9]) local-function-key-map))

(defun post-load-stuff ()
  (interactive)
  (w32-send-sys-command 61488)
  )
(add-hook 'window-setup-hook 'post-load-stuff t)

;; Stop Emacs from losing undo information by
;; setting very high limits for undo buffers
(setq undo-limit 20000000)
(setq undo-strong-limit 40000000)

;; Turning Off Annoying Sound
(setq visible-bell 1)

;; Setting Up Code Stuff

(setq jakub-makescript "build.bat")
(setq jakub-exe "w:/RoqueLike/build/RoqueLike.exe")


(require 'package)
(add-to-list 'package-archives '("melpa" . "http://melpa.milkbox.net/packages/"))
(package-initialize)

(require 'auto-complete)
(require 'auto-complete-config)
(ac-config-default)

;; (require 'yasnippet)
;; (yas-global-mode 1)

(require 'auto-complete-c-headers)
(require 'php-mode)
(require 'compile)

(load-library "view")
(require 'ido)
(ido-mode t)

(semantic-mode 1)

(require 'csharp-mode)
(require 'haskell-mode)
(require 'ruby-mode)
(require 'thingatpt)
(require 'imenu)

(defun my:add-semantic-to-autocomplete ()
  (add-to-list 'ac-sources 'ac-source-semantic)
  )
(add-hook 'c-mode-common-hook 'my:add-semantic-to-autocomplete)

(defun copy-line (arg)
  "Copy lines (as many as prefix argument) in the kill ring"
  (interactive "p")
  (kill-ring-save (line-beginning-position)
		  (line-beginning-position (+ 1 arg)))
  (message "%d line%s copied" arg (if (= 1 arg) "" "s"))
  )

;; Navigation
(defun previous-blank-line ()
  "Moves to the previous line containing nothing but whitespace."
  (interactive)
  (if (search-backward-regexp "^[ \t]*\n" nil t)
      nil
    (beginning-of-buffer)
    )
  )

(defun next-blank-line ()
  "Moves to the next line containing nothing but whitespace."
  (interactive)
  
  (forward-line)
  (if (search-forward-regexp "^[ \t]*\n" nil t)
      (forward-line -1)
    (end-of-buffer)
    )
  )

(defun define-key-for-code (key func)
  (define-key c++-mode-map key func)
  (define-key c-mode-map key func)
  (define-key csharp-mode-map key func)
  (define-key emacs-lisp-mode-map key func)
  (define-key org-mode-map key func)
  (define-key ruby-mode-map key func)
  )

(defun duplicate-line()
  (interactive)
  (move-beginning-of-line 1)
  (kill-line)
  (yank)
  (open-line 1)
  (next-line 1)
  (yank)
  )
					;alist of 'buffer-name / timer' items
(defvar buffer-tail-alist nil)

(defun buffer-tail (name)
  "follow buffer tails"
  (cond ((or (equal (buffer-name (current-buffer)) name)
	     (string-match "^ \\*Minibuf.*?\\*$" (buffer-name (current-buffer)))))
        ((get-buffer name)
	 (with-current-buffer (get-buffer name)
	   (goto-char (point-max))
	   (let ((windows (get-buffer-window-list (current-buffer) nil t)))
	     (while windows (set-window-point (car windows) (point-max))
		    (with-selected-window (car windows) (recenter -3)) (setq windows (cdr windows))))))))

(defun toggle-buffer-tail (name &optional force)
  "toggle tailing of buffer NAME. when called non-interactively, a FORCE arg of 'on' or 'off' can be used to to ensure a given state for buffer NAME"
  (interactive (list (cond ((if name name) (read-from-minibuffer 
					    (concat "buffer name to tail" 
						    (if buffer-tail-alist (concat " (" (caar buffer-tail-alist) ")") "") ": ")
					    (if buffer-tail-alist (caar buffer-tail-alist)) nil nil
					    (mapcar '(lambda (x) (car x)) buffer-tail-alist)
					    (if buffer-tail-alist (caar buffer-tail-alist)))) nil)))
  (let ((toggle (cond (force force) ((assoc name buffer-tail-alist) "off") (t "on")) ))
    (if (not (or (equal toggle "on") (equal toggle "off"))) 
	(error "invalid 'force' arg. required 'on'/'off'") 
      (progn 
        (while (assoc name buffer-tail-alist) 
	  (cancel-timer (cdr (assoc name buffer-tail-alist)))
	  (setq buffer-tail-alist (remove* name buffer-tail-alist :key 'car :test 'equal)))
        (if (equal toggle "on")
            (add-to-list 'buffer-tail-alist (cons name (run-at-time t 1 'buffer-tail name))))
        (message "toggled 'tail buffer' for '%s' %s" name toggle)))))

(defun my:launch-build ()
  (interactive)
  (start-process "main" "program_output" jakub-exe)
  
  (other-window -1)
  (switch-to-buffer "program_output")
  (toggle-buffer-tail "program_output" "on")
  (other-window -1)
  
  )

;;Line Movement Indentation
(defun my:next-line ()
  (interactive)
  (next-line)
  (indent-according-to-mode)
  )

(defun my:previous-line ()
  (interactive)
  (previous-line)
  (indent-according-to-mode)
  )

(defun my:move-beginning-of-line ()
  (interactive)
  (move-beginning-of-line 1)
  (indent-according-to-mode)
  )

(defun my:kill-line ()
  (interactive)
  (kill-line)
  (indent-according-to-mode)
  )

(defun my:open-line ()
  (interactive)
  (open-line 1)
  (my:next-line)
  (my:previous-line)
  (move-end-of-line 1)
  (indent-according-to-mode)
  )

(defun make-without-asking ()
  "Make the current build."
  (interactive)
  (save-buffer)
  (compile jakub-makescript)
  )

(defun ninja-make-without-asking ()
  "Make the current build."
  (interactive)
  (save-buffer)
  (compile (concat jakub-makescript " ninja"))
  )

(defun kill-buffer-other-window ()
  "Kills buffer In Other Window."
  (interactive)
  (other-window -1)
  (kill-buffer nil)
  (other-window -1)
  )

(defun casey-find-corresponding-file ()
  "find the file that corresponds to this one."
  (interactive)
  (setq CorrespondingFileName nil)
  (setq BaseFileName (file-name-sans-extension buffer-file-name))
  (if (string-match "\\.c" buffer-file-name)
      (setq CorrespondingFileName (concat BaseFileName ".h")))
  (if (string-match "\\.h" buffer-file-name)
      (if (file-exists-p (concat BaseFileName ".c")) (setq CorrespondingFileName (concat BaseFileName ".c"))
	(setq CorrespondingFileName (concat BaseFileName ".cpp"))))
  (if (string-match "\\.hin" buffer-file-name)
      (setq CorrespondingFileName (concat BaseFileName ".cin")))
  (if (string-match "\\.cin" buffer-file-name)
      (setq CorrespondingFileName (concat BaseFileName ".hin")))
  (if (string-match "\\.cpp" buffer-file-name)
      (setq CorrespondingFileName (concat BaseFileName ".h")))
  (if CorrespondingFileName (find-file CorrespondingFileName)
    (error "Unable to find a corresponding file")))

(defun casey-find-corresponding-file-other-window ()
  "Find the file that corresponds to this one."
  (interactive)
  (find-file-other-window buffer-file-name)
  (casey-find-corresponding-file)
  (other-window -1))

(defun ido-switch-buffer-other-window ()
  "Switches Buffer On The Other Window."
  (interactive)
  (other-window -1)
  (ido-switch-buffer)
  (other-window -1)
  )

(defun reload-emacs ()
  (interactive)
  (load-file ".emacs")
  )

(defun mine-goto-symbol-at-point ()
  "Will navigate to the symbol at the current point of the cursor"
  (interactive)
  (ido-goto-symbol (thing-at-point 'symbol)))

(defun ido-goto-symbol (&optional symbol-list)
  "Refresh imenu and jump to a place in the buffer using Ido."
  (interactive)
  (unless (featurep 'imenu)
    (require 'imenu nil t))
  (cond
   ((not symbol-list)
    (let ((ido-mode ido-mode)
	  (ido-enable-flex-matching
	   (if (boundp 'ido-enable-flex-matching)
	       ido-enable-flex-matching t))
	  name-and-pos symbol-names position)
      (unless ido-mode
	(ido-mode 1)
	(setq ido-enable-flex-matching t))
      (while (progn
	       (imenu--cleanup)
	       (setq imenu--index-alist nil)
	       (ido-goto-symbol (imenu--make-index-alist))
	       (setq selected-symbol
		     (ido-completing-read "Symbol? " symbol-names))
	       (string= (car imenu--rescan-item) selected-symbol)))
      (unless (and (boundp 'mark-active) mark-active)
	(push-mark nil t nil))
      (setq position (cdr (assoc selected-symbol name-and-pos)))
      (cond
       ((overlayp position)
	(goto-char (overlay-start position)))
       (t
	(goto-char position)))))
   ((listp symbol-list)
    (dolist (symbol symbol-list)
      (let (name position)
	(cond
	 ((and (listp symbol) (imenu--subalist-p symbol))
	  (ido-goto-symbol symbol))
	 ((listp symbol)
	  (setq name (car symbol))
	  (setq position (cdr symbol)))
	 ((stringp symbol)
	  (setq name symbol)
	  (setq position
		(get-text-property 1 'org-imenu-marker symbol))))
	(unless (or (null position) (null name)
		    (string= (car imenu--rescan-item) name))
	  (add-to-list 'symbol-names name)
	  (add-to-list 'name-and-pos (cons name position))))))))

(defun rotate-windows ()
  "Rotate your windows" 
  (interactive) 
  (cond 
   ((not (> (count-windows) 1))
    (message "You can't rotate a single window!"))
   (t
    (setq i 1)
    (setq numWindows (count-windows))
    (while  (< i numWindows)
      (let* (
	     (w1 (elt (window-list) i))
	     (w2 (elt (window-list) (+ (% i numWindows) 1)))
	     (b1 (window-buffer w1))
	     (b2 (window-buffer w2))
	     (s1 (window-start w1))
	     (s2 (window-start w2)))
	(set-window-buffer w1  b2)
	(set-window-buffer w2 b1)
	(set-window-start w1 s2)
	(set-window-start w2 s1)
	(setq i (1+ i)))))))

(defun jump-to-mark ()
  "Jumps To Mark" 
  (interactive)
  (exchange-point-and-mark 1)
  )

(defun my:tab-and-dabbrev ()
  "Jumps To Mark" 
  (interactive)
  (indent-for-tab-command)
  (dabbrev-expand)
  )


;; Keybindings

(global-set-key (kbd "M-p") 'find-file)
(global-set-key (kbd "M-P") 'find-file-other-window)
(global-set-key (kbd "M-s") 'save-buffer)
(global-set-key (kbd "M-o") 'other-window)
(global-set-key (kbd "M-k") 'kill-buffer)
(global-set-key (kbd "M-K") 'kill-buffer-other-window)
(global-set-key (kbd "M-u") 'ido-switch-buffer)
(global-set-key (kbd "M-U") 'ido-switch-buffer-other-window)
(global-set-key (kbd "M-l") 'recenter-top-bottom)
(global-set-key (kbd "M-i") 'set-mark-command)
(global-set-key (kbd "M-]") 'reload-emacs)
(global-set-key (kbd "M-}") 'delete-window)
(global-set-key (kbd "M-c") 'copy-line)
(global-set-key (kbd "H-i") 'duplicate-line)
(global-set-key (kbd "M-a") 'previous-blank-line)
(global-set-key (kbd "M-e") 'next-blank-line)
(global-set-key (kbd "M-'") 'rotate-windows)

(global-set-key (kbd "M-)") 'kmacro-end-and-call-macro)
(global-set-key (kbd "M-&") 'kmacro-start-macro)
(global-set-key (kbd "M-*") 'kmacro-end-macro)

(global-set-key (kbd "M-i") 'set-mark-command)
(global-set-key (kbd "M-I") (kbd "M-i M-i"))
(global-set-key (kbd "M-J") 'jump-to-mark)
;;(global-set-key (kbd "<backtab>") 'hs-toggle-hiding)

;;(global-set-key (kbd "<backtab>") 'my:tab-and-dabbrev)
(global-set-key (kbd "<backtab>") 'dabbrev-expand)

;; Code Ones !!!

(define-key-for-code (kbd "C-o") 'my:open-line)
(define-key-for-code (kbd "C-k") 'my:kill-line)
(define-key-for-code (kbd "C-a") 'my:move-beginning-of-line)
(define-key-for-code (kbd "C-p") 'my:previous-line)
(define-key-for-code (kbd "C-n") 'my:next-line)
(define-key-for-code (kbd "C-c C-c") 'comment-or-uncomment-region)

(define-key-for-code (kbd "M-q") 'query-replace)
(define-key-for-code (kbd "RET") 'newline-and-indent)
(define-key-for-code (kbd "M-z") 'next-error)
(define-key-for-code (kbd "M-Z") 'previous-error)
(define-key-for-code (kbd "M-a") 'previous-blank-line)
(define-key-for-code (kbd "M-e") 'next-blank-line)
(define-key-for-code (kbd "M-l") 'recenter-top-bottom)

(define-key-for-code (kbd "M-m") 'ninja-make-without-asking)
(define-key-for-code (kbd "M-M") 'make-without-asking)
(define-key-for-code (kbd "M-n") 'casey-find-corresponding-file)
(define-key-for-code (kbd "M-N") 'casey-find-corresponding-file-other-window)
(define-key-for-code (kbd "M-r") 'my:launch-build)
(define-key-for-code (kbd "C-m") 'ido-goto-symbol)

;; Org Mode !!!

(define-key org-mode-map (kbd "C-j") 'org-return)  
(define-key org-mode-map (kbd "C-p") 'previous-line)  
(define-key org-mode-map (kbd "C-n") 'next-line)
(define-key org-mode-map (kbd "C-y") 'yank)
(define-key org-mode-map (kbd "M-a") 'previous-blank-line)
(define-key org-mode-map (kbd "M-e") 'next-blank-line)

;; Ruby Mode !!

(defun run-current-file ()
  (interactive)
  (let* (
	 (ξsuffixMap
	  ;; (‹extension› . ‹shell program name›)
	  `(
	    ("php" . "php")
	    ("pl" . "perl")
	    ("py" . "python")
	    ("py3" . ,(if (string-equal system-type "windows-nt") "c:/Python32/python.exe" "python3"))
	    ("rb" . "ruby")
	    ("js" . "node") ; node.js
	    ("sh" . "bash")
	    ("clj" . "java -cp /home/xah/apps/clojure-1.6.0/clojure-1.6.0.jar clojure.main")
	    ("ml" . "ocaml")
	    ("vbs" . "cscript")
	    ("hs" . "runhaskell")
	    ;; ("pov" . "/usr/local/bin/povray +R2 +A0.1 +J1.2 +Am2 +Q9 +H480 +W640")
	    ))
	 (ξfName (buffer-file-name))
	 (ξfSuffix (file-name-extension ξfName))
	 (ξprogName (cdr (assoc ξfSuffix ξsuffixMap)))
	 (ξcmdStr (concat ξprogName " \""   ξfName "\"")))
    
    (when (buffer-modified-p)
      (save-buffer))
    
    (if (string-equal ξfSuffix "el") ; special case for emacs lisp
	(eval-buffer nil (get-buffer-create "*Shell Output*"))
      (if ξprogName
	  (progn
	    ;;(message "Running…")
	    (shell-command ξcmdStr "*Shell Output*"))
	(message "No recognized program file suffix for this file.")))
    (other-window -1)
    (switch-to-buffer "*Shell Output*")
    (other-window -1)
    )
  )


(defun my:ruby-mode-hook ()
  (define-key ruby-mode-map (kbd "M-m") 'run-current-file)
  )

(add-hook 'ruby-mode-hook 'my:ruby-mode-hook)

(defun my:elisp-mode-hook ()
  (define-key emacs-lisp-mode-map (kbd "M-m") 'run-current-file)
  )

(add-hook 'emacs-lisp-mode-hook 'my:elisp-mode-hook)

(defun my:haskell-mode-hook ()
  (define-key haskell-mode-map (kbd "M-m") 'run-current-file)
  )

(add-hook 'haskell-mode-hook 'my:haskell-mode-hook)
(add-hook 'haskell-mode-hook 'turn-on-haskell-indentation)

;;(remove-hook 'haskell-mode-hook 'turn-on-haskell-indent)
;;(add-hook 'haskell-mode-hook 'haskell-indent-mode)



;; Web Mode !!

(require 'web-mode)
(add-to-list 'auto-mode-alist '("\\.phtml\\'" . web-mode))
(add-to-list 'auto-mode-alist '("\\.tpl\\.php\\'" . web-mode))
(add-to-list 'auto-mode-alist '("\\.jsp\\'" . web-mode))
(add-to-list 'auto-mode-alist '("\\.as[cp]x\\'" . web-mode))
(add-to-list 'auto-mode-alist '("\\.erb\\'" . web-mode))
(add-to-list 'auto-mode-alist '("\\.mustache\\'" . web-mode))
(add-to-list 'auto-mode-alist '("\\.djhtml\\'" . web-mode))

(defun my-web-mode-hook ()
  "Hooks for Web mode."
  (setq web-mode-markup-indent-offset 2)
  (setq web-mode-css-indent-offset 2)
  (setq web-mode-code-indent-offset 2)
  )

(add-hook 'web-mode-hook  'my-web-mode-hook)

(defun my:ac-c-header-init ()
  ;;(require 'hs-minor-mode)
  (add-to-list 'ac-sources 'ac-source-c-headers)
  (add-to-list 'achead:include-directories '"C:/Program Files (x86)/Microsoft Visual Studio 12.0/VC/include")
  (add-to-list 'achead:include-directories '"D:/Libs/SFML-Visual_Studio2013x32/include/SFML")
  
  ;; 4-space tabs
  ;;(setq tab-width 4 indent-tabs-mode nil)
  (dolist (command '(yank yank-pop))
    (eval `(defadvice ,command (after indent-region activate)
	     (and (not current-prefix-arg)
		  (member major-mode '(emacs-lisp-mode lisp-mode
						       clojure-mode    scheme-mode
						       haskell-mode    ruby-mode
						       rspec-mode      python-mode
						       c-mode          c++-mode
						       objc-mode       latex-mode
						       plain-tex-mode))
		  (let ((mark-even-if-inactive transient-mark-mode))
		    (indent-region (region-beginning) (region-end) nil))))))
  )

(add-hook 'c++-mode-hook 'my:ac-c-header-init)
(add-hook 'c-mode-hook 'my:ac-c-header-init)
;;(add-hook 'prog-mode-hook #'hs-minor-mode)
(defun my:code-mode-hook ()
  ;; Dupa Narazie
  "Hooks for Code mode."
  (setq c-basic-offset 2)
  )

(add-hook 'c++-mode-hook 'my:code-mode-hook)
(add-hook 'c-mode-hook 'my:code-mode-hook)
(add-hook 'csharp-mode-hook 'my:code-mode-hook)

;; Treat h files as c++ based
(add-to-list 'auto-mode-alist '("\\.h\\'" . c++-mode))


(defun my:c++-mode-hook ()
  (c-set-offset 'arglist-intro '+)
  ;;(setq c-basic-offset 4)
  (c-set-offset 'substatement-open 0))
(add-hook 'c++-mode-hook 'my:c++-mode-hook)

;; Bright-red TODOs
(setq fixme-modes '(c++-mode c-mode emacs-lisp-mode))
(make-face 'font-lock-fixme-face)
(make-face 'font-lock-study-face)
(make-face 'font-lock-important-face)
(make-face 'font-lock-note-face)
(mapc (lambda (mode)
	(font-lock-add-keywords
	 mode
	 '(("\\<\\(TODO\\)" 1 'font-lock-fixme-face t)
	   ("\\<\\(STUDY\\)" 1 'font-lock-study-face t)
	   ("\\<\\(IMPORTANT\\)" 1 'font-lock-important-face t)
	   ("\\<\\(NOTE\\)" 1 'font-lock-note-face t))))
      fixme-modes)
(modify-face 'font-lock-fixme-face "Red" nil nil t nil t nil nil)
(modify-face 'font-lock-study-face "Yellow" nil nil t nil t nil nil)
(modify-face 'font-lock-important-face "Yellow" nil nil t nil t nil nil)
(modify-face 'font-lock-note-face "Dark Green" nil nil t nil t nil nil)

;; setq
(setq compilation-context-lines 0)
(setq compilation-error-regexp-alist
      (cons '("^\\([0-9]+>\\)?\\(\\(?:[a-zA-Z]:\\)?[^:(\t\n]+\\)(\\([0-9]+\\)) : \\(?:fatal error\\|warnin\\(g\\)\\) C[0-9]+:" 2 3 nil (4))
	    compilation-error-regexp-alist))

