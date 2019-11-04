(!) Explicațiile din README trebuie coroborate comentariilor ce însoțesc codul,
pentru a putea înțelege în totalitate implementarea.

Voi încerca să detaliez cât mai simplu implementarea în categoriile următoare:

1. Fișiere auxiliare
Arhiva cuprinde fișierele header.h și queue.h. Primul dintre acestea cuprinde două
structuri auxiliare, una pentru pachet - my_pkt, și una pentru buffer-ul din 
receiver - pkt_buff - în care, pe lângă pachet a fost reținută dimensiunea din
câmpul len al mesajului, pentru a avea datele corecte în ceea ce privește lungimea
șirului din câmpul de payload. Mai există tot aici și funcția de creare a unui
checksum, prin XOR pe toate caractere unui șir. Rezultatul este reținut într-un
câmp dedicat în pachet, care este comparat cu rezultatul recalculării în receiver.
Fișierul al doilea definește structura unei cozi (pentru fereastră) și funcțiile ce
pot fi aplicate pe aceasta.

2. Modul de implementare a sender-ului
Întâi, pe baza argumentelor în linia de comandă, este calculată dimensiunea ferestrei,
este deschis fișierul, este calculată dimensiunea acestuia, așa cum s-a studiat și în 
laborator. Cele trei detalii vor fi trimise într-un prim pachet, prin concatenarea în 
câmpul de payload al acestuia. S-a folosit ca delimitator '/', deoarece acest caracter
nu poate apărea în denumirile fișierelor pe Windows sau Unix. S-a stabilit o codificare
pentru tipul pachetelor (explicată în fișierul header.h). 
După trimiterea acestui prim pachet, se va trimite un calup care să asigure umplerea
ferestrei cu mesaje, unde datele vor fi citite din fișier.
Flow-ul programului a fost de aici rupt în două situații: toate mesajele trimise PÂNĂ
la dimensiunea ultimei ferestre au folosit pentru confirmare funcția clasică de receive,
fără timeout, pentru a eficientiza programul. În cazul în care un pachet este pierdut, 
situația este identificată pe parcurs într-un mod relativ simplu: se va primi un ack 
pentru un pachet diferit de primul din coadă. În acest caz, pe lângă trimiterea unui 
mesaj nou, se va retrimite și primul din coadă. La reordonare, în situația în care se va
trimite același pachet în mai multe mesaje, se va face eliminarea la receiver a instanțelor
următoare. Apare însă o nouă problemă: dacă se pierde fix ultimul mesaj? Nu mai există
un altul care să se afle după el în coadă, situație în care, așa cum am și identificat
în numeroase rulări, programul va îngheța: se așteaptă confirmare pentru un pachet care
nu a ajuns niciodată. Aici apare al doilea mod de receptare a pachetelor, prin timeout.
Acesta este aplicat doar pentru ultima fereastră (când se trimite și ultimul mesaj citit
din fișier). Astfel, marcând ca true (1) variabila 'active_timeout', doar pe lungimea
ultimei ferestre de mesaje trimise, se va folosi funcția de receive cu timeout (setat
la 2 * valoarea de delay primită ca parametru). În cazul timeout-ului, se va retrimite
primul mesaj din fereastră.
Situațiile de mesaje corupte (nak) au fost tratate simplu: primul mesaj din fereastră
este retrimis și mutat la sfârșitul cozii/ferestrei.

3. Modul de implementare a receiver-ului
Țințând cont că acesta este deschis atâta vreme cât nu a primit toate mesajele până la
dimensiunea totală a fișierului și că sender-ul se asigură că mesajele sunt trimise până
când toate sunt confirmate, nu a mai fost nevoie de tratarea vreunei situații de timeout
în receiver. Astfel, într-un while, până la primirea unei dimensiuni totale de bytes egală
cu cea a fișierului (primită în primul mesaj recepționat), se vor primi pachete. În cazul
unui mesaj corupt, se va trimite nak. Altfel, dacă mesajul cu acest număr de secvență nu a
mai fost primit (verificăm dacă valoarea câmpului len din buffer din această poziție este
zero), acesta va fi adăugat în buffer. Poziția este calculată ca numărul de secvență minus
numărul mesajelor listate în fișierul de OUT până în prezent, astfel încât, după scrierea
în fișier a primului rând de date din buffer, noile mesaje să ocupe mereu primele poziții.
Verificarea secvenței consecutive de mesaje (listăm în fișier doar atunci când nu avem 
goluri de mesaje în buffer, cauzate de reordonare, timeout, pierderi) a fost realizată
printr-o formulă simplă: numărul maxim de secvență din buffer minus numărul minim trebuie să
fie egal cu numărul de mesaje din buffer minus unu. Astfel, impunând ca primul minim să fie
zero (corespunzător primului mesaj, cel care conține informațiile despre fișiere), nu se va
încerca listarea în fișier, pentru că încă nu s-a primit numele fișierului de ieșire, deci
nu a fost încă deschis. Mai mult, pentru a eficientiza, chiar și în situația identificării
unei secvențe consecutive, se va amâna listarea până în ultimul moment: acela în care nu am
mai putea primi dimensiunea încă unei ferestre (dorim să evităm situația în care buffer-ul
are dimensiune 1000, fereastra 300, am primit 900 de mesaje, nu listăm în fișier, primim
mesajul cu număr de secvență 1100 și nu mai avem spațiu în buffer pentru a-l reține). Abia
atunci (sau când s-a atins EOF) se va face listarea. De menționat că, pentru prima listare,
poziția de început va fi unu (nu dorim să listăm în fișier conținutul primului mesaj,
referitor la informațiile despre fișiere și fereastră). După listare, buffer-ul este golit,
minimul devine numărul de secvență al următorului mesaj din următoarea fereastră (ultimul
număr de secvență din buffer + 1), iar maximul pleacă de la zero, urmând să fie modificat
pe măsură ce se va primi următorul set de mesaje. Pentru prima instanță necoruptă a fiecărui
mesaj, se va trimite ack, ceea ce va determina eliminarea acestuia din fereastra sender-ului.
Când am listat în fișier un număr de bytes egal cu dimensiunea fișierului primită prin 
mesajul cu număr de secvență zero, considerăm primirea ca încheiată, închizând fișierul de
ieșire.

Din ce se observă, toate cerințele au fost gândite integrat, de la bun început programul fiind
structurat pentru a rezolva toate tipurile de cerințe din enunț.