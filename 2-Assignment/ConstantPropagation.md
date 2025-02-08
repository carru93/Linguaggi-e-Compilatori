# Constant Propagation

## Spiegazione

L’obiettivo della constant propagation è quello di determinare in quali punti del programma le variabili hanno un valore costante.
L’informazione da calcolare per ogni nodo X del CFG è un insieme di coppie del tipo <variabile, valore costante>.
Se abbiamo la coppia <v, c> al nodo X, significa che v è garantito avere il valore c ogni volta che X viene raggiunto durante l’esecuzione del programma.

| Decisione              | Scelta                                                                                                             |
| ---------------------- | ------------------------------------------------------------------------------------------------------------------ |
| Domain                 | L’insieme delle possibili coppie <v, c>                                                                            |
| Direction              | Analisi forward                                                                                                    |
| Transfer Function      | OUT[B] = GEN[B] ∪ (IN[B] - KILLS[B])                                                                               |
| Meet Operator          | Intersezione (∩) (poiché il valore della costante deve essere lo stesso a prescidere dal percorso di provenienza). |
| Boundary Conditions    | Per il blocco di uscita: OUT[entry] = ∅                                                                            |
| Initial Interior Point | Inizialmente, per tutti gli altri blocchi, si assume OUT[B] = D.                                                   |

## Codice di esempio

```
k = 2
if (cond) {
   a = k + 2
   x = 5
} else {
    a = k * 2
    x = 8
}
k = a
while (cond) {
    b = 2
    x = a + k
    y = a * b
    k++
}
print(a + x)
```

## Basic Blocks

```
BB1: entry
BB2: k = 2
true: {
    BB3: a = k + 2
    BB4: x = 5
}
false: {
    BB5: a = k * 2
    BB6: x = 8
}
BB7: k = a

while: {
    BB8: b = 2
    BB9: x = a + k
    BB10: y = a * b
    BB11: k++
}
BB12: print(a + x)
BB13: exit
```

### Definizione di GEN[B] e KILL[B]

|          |                                                                 |
| -------- | --------------------------------------------------------------- |
| GEN[B]   | le espressioni calcolabili come costanti in B.                  |
| KILLS[B] | se B definisce una variabile v, invalida tutte le coppie <v,c>. |

## GEN e KILLS TABLES

| BB    | GEN[B]      | KILLS[B]                   |
| ----- | ----------- | -------------------------- |
| Entry | ∅           | ∅                          |
| BB2   | {<k, 2>}    | {<k, 2>, <k, a>, <k, k+1>} |
| BB3   | {<a, k+2>}  | {<a, k+2>, <a, k\*2>}      |
| BB4   | {<x, 5>}    | {<x, 5>, <x, 8>, <x, a+k>} |
| BB5   | {<a, k\*2>} | {<k, 2>, <k, a>, <k, k+1>} |
| BB6   | {<x, 8>}    | {<x, 5>, <x, 8>, <x, a+k>} |
| BB7   | {<k, a>}    | {<k, 2>, <k, a>, <k, k+1>} |
| BB8   | {<b, 2>}    | {<b, 2>}                   |
| BB9   | {<x, a+k>}  | {<x, 5>, <x, 8>, <x, a+k>} |
| BB10  | {<y, a\*b>} | {<y, a\*b>}                |
| BB11  | {<k, k+1>}  | {<k, 2>, <k, a>, <k, k+1>} |
| BB12  | ∅           | ∅                          |
| Exit  | ∅           | ∅                          |

## Iterazione 1

| BB    | IN[B]                           | OUT[B]                          | Note                      |
| ----- | ------------------------------- | ------------------------------- | ------------------------- |
| Entry | ∅                               | ∅                               |                           |
| BB2   | ∅                               | {<k,2>}                         |                           |
| BB3   | {<k,2>}                         | {<a,4>,<k,2>}                   | <a,k+2> => <a,4>          |
| BB4   | {<a,4>,<k,2>}                   | {<a,4>,<k,2>,<x,5>}             |                           |
| BB5   | {<k,2>}                         | {<a,4>,<k,2>}                   | <a,k\*2> => <a,4>         |
| BB6   | {<a,4>,<k,2>}                   | {<a,4>,<k,2>,<x,8>}             |                           |
| BB7   | {<a,4>,<k,2>}                   | {<a,4>,<k,4>}                   | IN = OUT[BB4] ∩ OUT[BB6]  |
| BB8   | {<a,4>,<k,4>}                   | {<a,4>,<b,2>,<k,4>}             | IN = OUT[BB7] ∩ OUT[BB11] |
| BB9   | {<a,4>,<b,2>,<k,4>}             | {<a,4>,<b,2>,<k,4>,<x,8>}       | <x,a+k> => <x,8>          |
| BB10  | {<a,4>,<b,2>,<k,4>,<x,8>}       | {<a,4>,<b,2>,<k,4>,<x,8>,<y,8>} | <y,a\*b> => <y,8>         |
| BB11  | {<a,4>,<b,2>,<k,4>,<x,8>,<y,8>} | {<a,4>,<b,2>,<k,5>,<x,8>,<y,8>} | <k,k+1> => <k,5>          |
| BB12  | {<a,4>}                         | {<a,4>}                         | IN = OUT[BB7] ∩ OUT[BB11] |
| Exit  | {<a,4>}                         | {<a,4>}                         |                           |

## Iterazione 2

| BB    | IN[B]               | OUT[B]              | Note                           |
| ----- | ------------------- | ------------------- | ------------------------------ |
| Entry | ∅                   | ∅                   |                                |
| BB2   | ∅                   | {<k,2>}             |                                |
| BB3   | {<k,2>}             | {<a,4>,<k,2>}       |                                |
| BB4   | {<a,4>,<k,2>}       | {<a,4>,<k,2>,<x,5>} |                                |
| BB5   | {<k,2>}             | {<a,4>,<k,2>}       |                                |
| BB6   | {<a,4>,<k,2>}       | {<a,4>,<k,2>,<x,8>} |                                |
| BB7   | {<a,4>,<k,2>}       | {<a,4>,<k,4>}       |                                |
| BB8   | {<a,4>}             | {<a,4>,<b,2>}       | IN = OUT[BB7] ∩ OUT[BB11]      |
| BB9   | {<a,4>,<b,2>}       | {<a,4>,<b,2>}       | <x,a+k> non è costante         |
| BB10  | {<a,4>,<b,2>}       | {<a,4>,<b,2>,<y,8>} | <y,a\*b> non è invalidato da k |
| BB11  | {<a,4>,<b,2>,<y,8>} | {<a,4>,<b,2>,<y,8>} |                                |
| BB12  | {<a,4>}             | {<a,4>}             | IN = OUT[BB7] ∩ OUT[BB11]      |
| Exit  | {<a,4>}             | {<a,4>}             |                                |

Possiamo osservare come l'arco back in uscita da BB11 abbia invalidato la variabile k, rendendo impossibile la propagazione della costante k=5 in BB8. Dopo la seconda iterazione l'analisi converge.
