# Tema 4 PCom - Protocolul HTTP. Client web.
Ca punct de start in rezolvare temei a servit **Laboratorul 09**, prin folosirea
fisierelor `buffer.c`, `buffer.h`, `helpers.c`, `helpers.h`, `requests.c`,
`requests.h`.
Cum tema a fost rezolvata in **C**, pentru lucru cu fisierele **JSON** m-am folosit
de biblioteca `parson.c`, sugerata si in enuntul temei (https://github.com/kgabis/parson).

### Flow-ul
Se primesc comenzi de la tastatura, pana la intalnirea comenzii de *exit*.

- `exit`: opreste programul si dezaloca memoria alocata la inceputul executiei;
- `register`:
    - verifica daca exita deja o sesiune activa => afisarea unei erori corespunzatoare;
    - se deschide o conexiune cu serverul;
    - se introduce un username si o parola;
    - se construieste un obiect **JSON** care e ulterior trimis catre server ca un **POST**
        request;
    - se verifica raspunsul primit de la server pentru a afisa un mesaj corespunzator situatiei;
- `login`:
    - se verifica daca exista o sesiune activa => afisarea unui mesaj corespunzator;
    - se deschide o conexiune cu serverul;
    - se introduce un username si o parola;
    - se creaza un obiect **JSON** care e trimit catre server sub forma de **POST** request;
    - se afiseaza un mesaj corespunzator raspunsului serverului;
    - in caz de succes, se extrag cookie-urile din raspunsul serverului;
- `enter_library`:
    - verifica daca utilizatorul e logat prin verificarea existentei cookie-urilor primite
        in urma logarii;
    - se deschide o conexiune cu serverul;
    - se cere accesul la biblioteca printr-un **GET** request;
    - in functie de raspunsul serverului, se afiseaza un mesaj corespunzator;
    - se extrage token-ul **JWT** din raspunsul serverului;
- `get_books`:
    - verifica daca utilizatorul e logat sau daca are acces la biblioteca; pentru fiecare caz
        se afiseaza un mesaj corespunzator;
    - se deschide conexiunea cu serverul;
    - se efectueaza un **POST** request pentru obtinerea tuturor cartilor;
    - daca s-a intampinat esec, se afiseaza un mesaj care descrie situatia si se asteapta
        urmatoarea comanda, altfel, folosind functiile pentru formatul **JSON**, se extrge
        array-ul de carti din raspunsul serverului; se afiseaza, intr-un loop, cartile;
- `get_book`:
    - verifica daca utilizatorul e logat sau daca are acces la biblioteca; pentru fiecare caz
        se afiseaza un mesaj corespunzator;
    - se deschide o conexiune cu serverul;
    - se introduce *id*-ul cartii cautate;
    - se verifica validitatea *id*-ului introdus;
    - pentru un *id* invalid, se opreste comanda, se inchide conexiunea si se asteapta executia
        urmatoarei comenzi;
    - se realizeaza o cerere **GET** catre server;
    - daca cartea nu a fost gasita, se afiseaza eroare, altfel, cu ajutorul functiilor **JSON**,
        se extrag toate campurile cartii si se afiseaza la **STDOUT**;
- `add_book`:
    - verifica daca utilizatorul e logat sau daca are acces la biblioteca; pentru fiecare caz
        se afiseaza un mesaj corespunzator;
    - se deschide o conexiune cu serverul;
    - se introduc toate detaliile cartii;
    - se verifica validitatea numarului de pagini a cartii;
    - se creaza un obiect **JSON** care e trimis ulterior spre server;
    - se interpreteaza raspunsul serverului;
- `delete_book`:
    - verifica daca utilizatorul e logat sau daca are acces la biblioteca; pentru fiecare caz
        se afiseaza un mesaj corespunzator;
    - se deschide o conexiune cu serverul;
    - se introduce *id*-ul cartii;
    - se verifica validitatea *id*-ului;
    - se efectueaza o cerere **GET_DELETE**;
    - se interpreteaza raspunsul serverului;
- `logout`:
    - se verifica ca utilizatorul sa fie logat;
    - se trimite cerere de delogare, de tip **GET**, catre server;
    - se afseaza un mesaj la **STDOUT** in functie de raspunsul serverului;
- alte comenzi primite de la tastatura duc la afisarea mesajului de *Invalid Command.*.
