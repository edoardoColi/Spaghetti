# Note di Implementazione – by Me and ChatGPT
Crediti anche per [Brackeys](https://youtu.be/LOhfqjmasi0?si=NP10GlYgAc316hT4).

## Normalizzazione dell’input da tastiera

L’input proveniente dalla tastiera viene **normalizzato** per garantire movimenti consistenti.
In questo modo, quando il giocatore preme più tasti direzionali insieme, la velocità risultante rimane costante (ad esempio muovendosi in diagonale non risulta più veloce che muoversi in orizzontale o verticale).

---

## Gestione delle animazioni

Le animazioni possono essere gestite in due modi:

* Con `AnimatedSprite2D`, utile per gestire più animazioni in maniera strutturata.
* Con `Sprite2D`, quando è sufficiente cambiare texture o frame manualmente.

Questa flessibilità permette di adattare l’implementazione alle esigenze di ciascun asset.

---

## Sistema di collisioni del taxi

Il taxi utilizza **due CollisionShape2D** per un rilevamento più accurato.
Inoltre, i bordi della mappa sono definiti come **rette infinite invalicabili**, così da delimitare lo spazio di gioco senza bisogno di disegnare muri fisici.

---

## Gestione dei file importati

Non tutti i file importati vengono necessariamente utilizzati.
Vengono comunque caricati nel progetto per permettere in futuro un **restyle rapido** o la sostituzione di asset senza modificare il codice di base.

---

## Accelerazione e frenata non istantanee

Le azioni di accelerazione e frenata non avvengono in modo immediato, ma seguono una **curva graduale**.
Questo rende la fisica del veicolo più realistica e piacevole da controllare.

---

## Camera come figlia del Player

La camera è stata collegata direttamente al nodo del giocatore (`Player` → `Camera2D`).
In questo modo, la visuale **segue automaticamente** i movimenti del personaggio senza bisogno di codice aggiuntivo.

---

## Ordinamento degli asset (Z-index e layering)

Il motore disegna i nodi **dall’alto verso il basso** nell’albero della scena.
Per casi specifici (es. case con tetto davanti e porta dietro) conviene usare lo **Z-index** nel menu *Ordering*.
Lo Z-index permette di gestire facilmente quali elementi stanno davanti o dietro, senza dipendere rigidamente dalla struttura ad albero dei nodi.

---

## Gestione delle collisioni con layer e mask

Per migliorare la gestione delle interazioni:

* Gli ostacoli e le monete non entrano direttamente come figli di ogni entità.
* Viene definito un **layer di collisione separato** tra giocatore e coin/ostacolo.
* Tramite la **collision mask**, il coin/ostacolo può rilevare solo il giocatore, senza influenzare altri oggetti.

Questa separazione semplifica il controllo e riduce conflitti tra diversi tipi di entità.

---

## Gestione della UI

Durante la pausa, i nodi si fermano per impostazione predefinita. Per rendere interattivo il **PauseMenu**, impostare la proprietà `pause_mode` dei pulsanti su **Always**.

---

## Gestione della pausa

`get_tree().paused = true` sospende tutto, inclusi i pulsanti. Per farli funzionare comunque, in Inspector sotto **Process**, impostare su **Always**.

---
