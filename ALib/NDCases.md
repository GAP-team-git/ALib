#  Principali di operazioni ND

| #  | Operazione                                   | Shape Input                                        | Shape Output | Commento                                         |
| -- | -------------------------------------------- | -------------------------------------------------- | ------------ | ------------------------------------------------ |
| 1  | Somma totale (`sum_nd`)                      | (3,4)                                              | (1)          | Riduce tutti gli assi → scalares                 |
| 2  | Somma su asse 1 (`sum_nd(slice_view, {1})`)  | (2,4)                                              | (2)          | Riduce colonne, mantiene righe                   |
| 3  | Max su asse 0 (`max_nd(img,{0})`)            | (3,4)                                              | (4)          | Massimo per ogni colonna                         |
| 4  | Min su assi 0 e 1 (`min_nd(img,{0,1})`)      | (3,4)                                              | (1)          | Riduce tutto → scalar                            |
| 5  | Media su tutti gli assi (`mean_nd(img)`)     | (3,4)                                              | (1)          | Scalar, somma/size                               |
| 6  | Argmax ND (`argmax_nd(img)`)                 | (3,4)                                              | (2)          | Restituisce indice multidimensionale `[row,col]` |
| 7  | Argmin ND (`argmin_nd(img)`)                 | (3,4)                                              | (2)          | Restituisce indice multidimensionale `[row,col]` |
| 8  | Slice view (`slice({0,0},{2,4})`)            | (3,4)                                              | (2,4)        | Non copia dati, solo view                        |
| 9  | Transpose (`transpose()`)                    | (3,4)                                              | (4,3)        | Modifica strides e dims, view senza copia        |
| 10 | Broadcast + reduce (`sum_nd(trans+im2,{0})`) | trans: (4,3), im2: (3,4) → broadcast a compatibile | (4)          | Broadcasting automatico, riduce asse 0           |

🔹 Note chiave dalla tabella
Output shape dipende dagli assi ridotti (axes)
Slice / transpose restituiscono view → zero copie
Broadcasting lavora automaticamente grazie a ATensorIterator
Argmax / Argmin ND restituiscono sempre indice multidimensionale
Per reduce totale, l’output è sempre scalar (shape {1})

🔹 ND Array – Guida rapida
1️⃣ Creazione array
AArray<float> img({3,4});   // 3 righe, 4 colonne
AArray<float> im2({3,4});

2️⃣ Lazy operations (no copia)
auto expr = img + im2;      // AExpr
auto lazy_mul = img * 2.0f; // scalares o broadcast
Non materializza i dati fino a eval()
Puoi concatenare più operazioni

3️⃣ Materializzazione (Eval)
AArray<float> result = expr.eval();
Restituisce un AArray concreto
Necessario per passare dati a funzioni esterne o output

4️⃣ Slice / View
auto slice_view = img.slice({0,0},{2,4}); // prime due righe
auto trans_view = img.transpose();         // 4x3 view, non copia
View non copia dati
Mantiene stride e offset
Può essere usata in qualsiasi reduce ND

5️⃣ Reduce ND
Somma
auto sum_total = sum_nd(img);       // scalares, riduce tutti gli assi
auto sum_axis1 = sum_nd(img, {1});  // somma colonne, shape = (3)
auto sum_axis0 = sum_nd(img, {0});  // somma righe, shape = (4)
Massimo / Minimo
auto max0 = max_nd(img, {0});       // max per ogni colonna
auto min01 = min_nd(img, {0,1});    // min totale → scalar
Media
auto mean_all = mean_nd(img);         // scalar
auto mean_axis1 = mean_nd(img, {1});  // media righe

6️⃣ Argmax / Argmin ND
auto idx_max = argmax_nd(img); // restituisce {row, col}
auto idx_min = argmin_nd(img); // restituisce {row, col}
Sempre indice ND
Per array 3x4 → {0..2,0..3}

7️⃣ Broadcast automatico
AArray<float> a({3,1});
AArray<float> b({1,4});
auto sum_bcast = sum_nd(a + b, {0}); // a e b broadcast a shape compatibile
Funziona anche con slice / transpose
ATensorIterator gestisce stride e offset

8️⃣ Esempio completo combinato
auto slice2 = img.slice({0,0},{2,4});
auto expr2  = slice2 + im2.transpose();
auto result = sum_nd(expr2, {0});    // somma lungo primo asse
auto idx_max = argmax_nd(expr2);
Lazy expression → eval solo se serve
Slice + transpose + broadcasting → tutto senza copiare dati
Reduce ND → shape output coerente con assi ridotti

💡 Tip:

Usa sempre eval() solo quando devi materializzare il risultato.
Per operazioni in-place, puoi scrivere:
img += im2; // usa lazy evaluation e inplace operator
Per aggregatori ND complessi (sum_nd, max_nd, argmax_nd) puoi combinare slice, transpose e broadcasting senza modificare il codice.
