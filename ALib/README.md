#  Error Management

             ┌─────────────┐
             │   Modulo    │
             │  (Array,    │
             │  AShape,    │
             │  Tensor...) │
             └─────┬───────┘
                   │ genera AErrorInfoEx
                   │
                   ▼
           ┌───────────────┐
           │ handleError() │
           └─────┬─────────┘
                 │
       ┌─────────┴──────────┐
       │                    │
       ▼                    ▼
Critical / Fatal       Warning / Recoverable
(severity == critical) (severity == warning/recoverable)
       │                    │
       │                    │
   ┌───────────┐         ┌───────────┐
   │  Throw    │         │  Log /    │
   │ AException│         │ printError│
   └─────┬─────┘         └─────┬─────┘
         │                      │
         ▼                      ▼
   try/catch in main       Flusso continua
   o modulo superiore
         │
         ▼
   Gestione centralizzata:
   - Logging avanzato
   - UI o messaggi all’utente
   - Retry / fallback
