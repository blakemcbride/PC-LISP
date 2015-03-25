(def SOCKETOPEN
  (lexpr (_N_)
    ((lambda (addr port time fd)
       (repeat 20
         (cond (time (setq fd (socketopen addr port time)))
           (t (setq fd (socketopen addr port))))
         (and fd (return fd))
         (sleep 0.5)))
     (arg 1)
     (arg 2)
     (arg? 3 nil)
     nil)))
(setq skt (SOCKETOPEN "*" 31270))
(and skt (close skt))
(prog nil
  (setq fp (SOCKETOPEN "*" 31274 60))
  (and fp (b-write (eval (car (b-read fp 10))) fp 10))
  (exit))
