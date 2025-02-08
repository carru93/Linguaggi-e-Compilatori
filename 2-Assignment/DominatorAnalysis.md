# Dominator Analysis

## Spiegazione

La dominator analysis è un'analisi che per ogni nodo di un Control Flow Graph, determina l’insieme dei nodi che dominano quel nodo. Si definisce che un nodo X domina un nodo Y se ogni percorso dall’entrata del CFG a Y passa necessariamente per X.

| Decisione              | Scelta                                                                            |
| ---------------------- | --------------------------------------------------------------------------------- |
| Domain                 | L’insieme dei nodi del CFG D={A, B, C, D, E, F, G}.                               |
| Direction              | Analisi forward                                                                   |
| Transfer Function      | OUT[B] = B ∪ IN[B]                                                                |
| Meet Operator          | Intersezione (∩) (perché un nodo domina n solo se domina ogni predecessore di n). |
| Boundary Conditions    | OUT[entry] = entry                                                                |
| Initial Interior Point | Inizialmente, per tutti gli altri blocchi, si assume OUT[B] = D.                  |

### Codice di esempio

```
         A
        / \
       B   C
       |  / \
       | D   E
       |  \ /
       |   F
       +-->G
```

### Definizione di GEN[B] e KILL[B]

|          |                                                                   |
| -------- | ----------------------------------------------------------------- |
| GEN[B]   | Ogni basic block genera sè stesso.                                |
| KILLS[B] | Ogni basic block non sovrascrivere o non elimina nessun altro BB. |

## GEN e KILLS TABLES

| BB  | GEN[B] | KILLS[B] |
| --- | ------ | -------- |
| A   | A      | ∅        |
| B   | B      | ∅        |
| C   | C      | ∅        |
| D   | D      | ∅        |
| E   | E      | ∅        |
| F   | F      | ∅        |
| G   | G      | ∅        |

## Iterazione 1

| BB  | IN[b] | OUT[b]  |
| --- | ----- | ------- |
| A   | ∅     | A       |
| B   | A     | A, B    |
| C   | A     | A, C    |
| D   | A, C  | A, C, D |
| E   | A, C  | A, C, E |
| F   | A, C  | A, C, F |
| G   | A     | A, G    |

## Conclusione

Data l'assenza di archi back, l'analisi termina con la prima iterazione.
