# Aggregatori ND

| Aggregatore        | Input shape           | Assi ridotti | Output shape    | Note / comportamento                       |
| ------------------ | --------------------- | ------------ | --------------- | ------------------------------------------ |
| `sum_nd`           | (M,N,…)               | {} (vuoto)   | (1)             | Somma tutto → scalar                       |
| `sum_nd`           | (M,N,…)               | {0}          | (N,…)           | Somma lungo primo asse (righe)             |
| `sum_nd`           | (M,N,…)               | {1}          | (M,…)           | Somma lungo secondo asse (colonne)         |
| `sum_nd`           | (M,N,…)               | {0,1}        | (1)             | Riduce assi 0 e 1 → scalar                 |
| `mean_nd`          | (M,N,…)               | {}           | (1)             | Media totale                               |
| `mean_nd`          | (M,N,…)               | {1}          | (M,)            | Media colonne                              |
| `max_nd`           | (M,N,…)               | {}           | (1)             | Massimo totale                             |
| `max_nd`           | (M,N,…)               | {0}          | (N,)            | Massimo per colonna                        |
| `min_nd`           | (M,N,…)               | {}           | (1)             | Minimo totale                              |
| `min_nd`           | (M,N,…)               | {1}          | (M,)            | Minimo per riga                            |
| `argmax_nd`        | (M,N,…)               | {}           | (rank)          | Indice ND del massimo                      |
| `argmin_nd`        | (M,N,…)               | {}           | (rank)          | Indice ND del minimo                       |
| Slice / view       | (M,N,…)               | –            | (subset)        | Non copia dati, mantiene stride            |
| Transpose          | (M,N,…)               | –            | (N,M,…)         | Cambia strides e dims, view                |
| Broadcast + reduce | (A shape compatibile) | {assi}       | (shape ridotto) | Gestito automaticamente da ATensorIterator |


#Flusso operativo ND

Input AArray (shape M×N×P…)

        │
        ▼
      Slice / View
        │
        ▼
   Transpose / Permute
        │
        ▼
   Lazy Expression (AExpr)
        │
        ▼
   Broadcasting
        │
        ▼
   Reduce ND (sum_nd, max_nd…)
        │
        ▼
   Output AArray / Scalar
(shape ridotto in base agli assi ridotti)

9️⃣ Suggerimenti rapidi
Usa eval() solo quando serve materializzare
Slice / transpose → zero copie, solo view
Operazioni lazy → concatenate senza allocazioni
Argmax / Argmin ND → restituiscono indice multidimensionale
Assi ridotti → specificare {} o {0,1,...} per il reduce
