# Very Busy Expressions

## Spiegazione

### Cos’è una very busy expression?

Una very busy expression è un’espressione che, a partire da un certo punto del programma, verrà necessariamente calcolata lungo ogni percorso fino all’uscita, prima che una delle sue variabili venga ridefinita.

| Decisione              | Scelta                                                                                |
| ---------------------- | ------------------------------------------------------------------------------------- |
| Domain                 | L’insieme D di tutte le espressioni D={(a-b), (b-a)}.                                 |
| Direction              | Analisi backward                                                                      |
| Transfer Function      | IN[B] = GEN[B] ∪ (OUT[B] - KILLS[B])                                                  |
| Meet Operator          | Intersezione (∩) (poiché l’espressione deve essere calcolata lungo tutti i percorsi). |
| Boundary Conditions    | Per il blocco di uscita: OUT[exit] = ∅                                                |
| Initial Interior Point | Inizialmente, per tutti gli altri blocchi, si assume IN[B] = D.                       |

### Codice di esempio

```c
if(a != b) {
    x = b - a
    x = a - b
} else {
    y = b - a
    a = 0
    x = a - b
}
```

### Basic Blocks

```
BB1: entry
BB2: a!=b?
true:
{
    BB3: x = b - a
    BB4: x = a - b
}
false:
{
    BB5: y = b - a
    BB6: a = 0
    BB7: x = a - b
}
BB8: exit
```

### Definizione di GEN[B] e KILL[B]

|          |                                                             |
| -------- | ----------------------------------------------------------- |
| GEN[B]   | le espressioni calcolate in B                               |
| KILLS[B] | le espressioni che contengono una variabile ridefinita in B |

## GEN e KILLS TABLES

| BB    | GEN[B]  | KILLS[B]      |
| ----- | ------- | ------------- |
| Entry | ∅       | ∅             |
| BB2   | ∅       | ∅             |
| BB3   | {(b-a)} | ∅             |
| BB4   | {(a-b)} | ∅             |
| BB5   | {(b-a)} | ∅             |
| BB6   | ∅       | {(a-b),(b-a)} |
| BB7   | {(a-b)} | ∅             |
| Exit  | ∅       | ∅             |

## Iterazione 1

| BB  | IN[B]         | OUT[B]  |
| --- | ------------- | ------- |
| BB1 | {(b-a)}       | {(b-a)} |
| BB2 | {(b-a)}       | {(b-a)} |
| BB3 | {(a-b),(b-a)} | {(a-b)} |
| BB4 | {(a-b)}       | ∅       |
| BB5 | {(b-a)}       | ∅       |
| BB6 | ∅             | {(a-b)} |
| BB7 | {(a-b)}       | ∅       |
| BB8 | ∅             | ∅       |

## Conclusione

Data l'assenza di archi back, l'analisi termina con la prima iterazione.
