(require 'generic-x) ;; we need this

(define-generic-mode 
    'oyster-mode                      ;; name of the mode
  '("#")                            ;; comments start with '#'
  '("::" "<-" "..." 
    "clear" "←" "λ" 
    "\\"  "==" "&&" "||" 
    "+" "-" "/" "*")        ;; some keywords
  '((";" . 'font-lock-operator))    ;; ';' is a a built-in 
  '("\\.oy$")                       ;; files for which to activate this mode 
  (list 'oyster-mode-setup)         ;; other functions to call
  "A mode for tainted oyster"       ;; doc string for this mode
)

(defun oyster-mode-setup ()
  (setq indent-line-function 'oyster-indent-line)
  (local-set-key (kbd "<backtab>") 'oyster-deindent-line))

(defun oyster-indent-line (&optional whole-exp)
  "Indent current line"
  (interactive)
  (let ((indent (oyster-correct-indentation))
        (pos (- (point-max) (point))) 
        beg)
    (beginning-of-line)
    (setq beg (point))
    (skip-chars-forward " ")
    (if (zerop (- indent (current-column)))
        nil
      (delete-region beg (point))
      (indent-to indent))
    (if (> (- (point-max) pos) (point))
	(goto-char (- (point-max) pos)))
    ))

(defun oyster-deindent-line (&optional whole-exp)
  "DEindent current line"
  (interactive)
  (let ((pos (- (point-max) (point))) 
        beg)
    (beginning-of-line)
    (setq beg (point))
    (skip-chars-forward " ")
    (if (or (oyster-impossible-deindent) (zerop (current-column)))
        nil
      (delete-region beg (+ beg 4)))
    (if (> (- (point-max) pos) (point))
	(goto-char (- (point-max) pos)))
    ))

(defun oyster-impossible-deindent () 
  (save-excursion
    (beginning-of-line)
    (skip-chars-backward "\n ")

    (if (eq (char-before) ?:)
        t)))

(defun oyster-correct-indentation ()
  (save-excursion
    (let (poo)

      (beginning-of-line)
      (skip-chars-backward "\n ")

      (if (eq (char-before) ?:)
          (setq poo 4)
        (setq poo 0))

      (beginning-of-line)
      (skip-chars-forward " ")
      (setq poo (+ poo (current-column)))

      poo)))