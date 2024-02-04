# Projekt zaliczeniowy
**BIGOS**  
**Programowanie I**  
**X 2023 - I 2024**  
**Kamil Bublij gr. 4/7**

## 1. Opis projektu
Celem projektu było poznanie różnych technik programistycznych języka C++, w szczególności za pośrednictwem funkcjonalności dostępnych w środowisku Win32 i złożenie ich w spójną całość.

## 2. Funkcjonalność
Zagadnienia poruszane w projekcie:
- elementy programowania obiektowego;
- biblioteka graficzna Direct2D, dokładniej DirectWrite;
- biblioteka graficzna GDI;
- dynamiczne zarządzanie pamięcią;
- kontenery (map, set, string, vector);
- wielowątkowość;
- szeroko rozumiane dialogi i kontrolki Win32;
- rozwinięty model rodzic-dziecko, w tym haki i dane dodatkowe okien;
- treści multimedialne (dźwięk, bitmapy);
- tworzenie przejrzystego interfejsu (menu, wstążki, skróty klawiszowe);
- weryfikacja danych wprowadzonych przez użytkownika;
- komendy preprocesora i konsolidatora;
- algorytmika (kodowania Huffmana).


## 3. Przebieg realizacji
Program jest zbudowany na zasadzie modułów - każda jego większa część ma własny plik źródłowy i osobny plik nagłówka.
### Spis plików i funkcjonalności
- `main.cpp`, `main.h` - główne pliki, zawierają podstawowe okno, instrukcje uruchamiania modułów i dialogu *O programie*;
- `GraphicsDemo.cpp`, `GraphicsDemo.h` - pobiera informacje o możliwościach graficznych systemu użytkownika, po czym je wyświetla;
-  `Trigonometry.cpp`, `Trigonometry.h` - moduł rysujący funkcje okresowe;
-  `Tree.cpp`, `Tree.h` - pobiera informacje o tekście (lub parametrach kodowania), po czym na ich podstawie rysuje drzewo kodowania;
- `Resources.rc`, `Resources.h` -– pliki zasobów. Nie zawierają kodu, a elementy statyczne potrzebne programowi do działania;
- `tada.wav`, inne pliki osadzone w `Resources.rc` - pliki multimedialne utworzone przeze mnie, użyte za zgodą autora lub będące częścią domeny publicznej.

### Zastosowane algorytmy
Wszystkie podane algorytmy używają struktury `LEAF` (odtąd nazywanej *liściem*) jako podstawowej jednostki:
```cpp
TYPEDEF STRUCT LEAF{
	UINT ID = NULL;
std::wstring tSymbol = {};
std::wstring tFPValue = {};
DOUBLE FPValue = 0;
UINT FFValue = 0;
STRUCT LEAF* LeftChild = nullptr;
STRUCT LEAF* RightChild = nullptr;
} LEAF, * LPLEAF;
```
Elementy `*LeftChild` i `*RightChild` będą nazywane (odpowiednio: lewymi i prawymi) *dziećmi*, a liście, w których się znajdują, są ich *rodzicami*.
#### I - Algorytm generowania optymalnego drzewa kodowania Huffmana
**Wymagania wstępne**
- dane wprowadzone do algorytmu są poprawne (program to zapewnia),
- istnieje wystarczająco miejsca na przechowanie liści pośrednich,
**Kroki**  
1.  Jeśli na liście pozostał jeden element, koniec
2.  Posortuj liście rosnąco według prawdopodobieństwa/częstości
3.  Wybierz dwa pierwsze liście listy (mają najmniejsze prawdopodobieństwo)
4.  Utwórz nowy liść, który jako dzieci ma liście wzięte w kroku `3.`,
    a jego prawdopodobieństwo jest równe sumie prawdopodobieństw 
    jego dzieci
5.  Usuń liście wzięte w kroku `2.` z listy
6.  Włóż liść otrzymany w kroku `3.` do listy
7.  Wróć do kroku `1.`

#### II -  Algorytm graficznie reprezentujący drzewo binarne
**Wymaganie wstępne**
- drzewo jest binarne (nie musi być kompletne), algorytm `I` to gwarantuje,
- drzewo ma niezerową długość,
**Kroki**
1. Zainicjalizuj pusty stos rodziców
2. Ustaw się na najstarszym liściu drzewa
3. Dopóki istnieje, rysuj lewe dziecko liścia i załaduj je; każde narysowane dziecko wrzuć na stos (powtarzaj)
4. Dopóki jesteś prawym dzieckiem, a stos nie jest pusty, cofnij się; usuń każde dziecko, z którego się cofasz, ze stosu (powtarzaj)
**Ważne:** Podczas przejścia z kroku `3.` do `4.` stos nigdy nie będzie pusty
5. Jeśli skończył się stos, koniec
6. Jesteś lewym dzieckiem, więc narysuj *jedno* prawe dziecko, przejdź do niego i wrzuć je na stos
7. Wróć do kroku `3.`

## 4. Instrukcja użytkownika
Program powinien być opatrzony ikoną, która, naturalnie, przedstawia kubek soku jabłkowego.
#### Menu główne
Użytkownik jest witany ekranem powitalnym, który mówi mu, co ma dalej zrobić.
![image](https://github.com/kamilok04/Win32Playground/assets/94155059/5c4c0709-2f2d-425b-8c75-a02e2508e650)  
Ku górze ekranu znajduje się menu, z którego można wybrać kilka pozycji:
*Notatka: Zgodnie z konwencją, wszystkie pozycje menu zakończone wielokropkiem (…) powodują otwarcie nowego okna.*
-	Plik:
	- Zakończ (<kbd>Alt</kbd> + <kbd>F4</kbd>) – kończy program.
- Opcje:
	- Tryb szybki (<kbd>F4</kbd>) – na żądanie włącza/wyłącza użycie biblioteki Direct2D.
	- <tylko skrót klawiszowy> (<kbd>Shift</kbd> + <kbd>F4</kbd>) – niszczy klasę Direct2D, zmniejszając zużycie pamięci około 10-krotnie.
-	„Fajne rzeczy”:
	-	Właściwości grafiki… - otwiera opisany wcześniej moduł GraphicsDemo.
	-	Pokaz trygonometrii… (<kbd>Ctrl</kbd> + <kbd>T</kbd>) – otwiera moduł Trigonometry.
	-	Drzewko… (<kbd>Ctrl</kbd> + <kbd>H</kbd>) – otwiera moduł Tree.
-	Pomoc:
	-	O programie… (<kbd>F1</kbd>) – wyświetla informacje o projekcie.
#### Moduł *GraphicsDemo*
![image](https://github.com/kamilok04/Win32Playground/assets/94155059/043021b5-a072-4589-84a9-149457de91ac)  
Moduł nie zawiera menu. Okno reaguje na przewijanie paskiem i kółkiem myszy, przy zmianie rozmiaru paski dostosowują się odpowiednio. Mały moduł; nie zawiera funkcji innych niż standardowe `WndProc`.  
#### Moduł *Trigonometry*  
![image](https://github.com/kamilok04/Win32Playground/assets/94155059/8f5cf654-4e08-45cc-82f0-294519f3ea52)   
Moduł zawiera klasyczne menu oraz wstążkę, która zawiera suwaki, służące do zmiany odpowiednio prędkości i przyspieszenia przesuwania się funkcji. 
Co ciekawe, funkcje odpowiednio skalują się wraz ze zmianą rozmiaru ekranu.

**Użyte funkcje pokazowe**
Funkcja | Wzór
---|---
Pokazowa 1 | $\frac{1}{2}\cdot(\cos(x)+\sin(2x)+\cos(3x))$
Pokazowa 2 | $\frac{1}{4}\cdot(\sin(x)+\cos(2x)-\cos(3x)-\cos(4x)-\sin(7x)+\sin(9x^2))$

Struktura menu:
-	Funkcje:
(wszystkie opcje odpowiadają za przełączanie wyświetlania funkcji)
	-	Sinus
	-	Cosinus - 
	-	Cocosinus - :)
	-	Tangens
	-	Cotangens
	-	Pokazowa 1
	-	Pokazowa 2
   
**Dialog *Zmień zakres…***  

![image](https://github.com/kamilok04/Win32Playground/assets/94155059/e7f75463-395e-45ae-9a82-eac30a47c4f5)  
Dialog składa się z kilku podokien. Podokno Wybór funkcji pozwala wybrać funkcję spośród tych, które są narysowane (żeby zmienić parametry funkcji, należy ją włączyć). Podokno Właściwości pozwala na dostosowanie precyzji rysowania, koloru oraz grubości kreski funkcji. Żeby zmienić kolor, należy użyć przycisku Zmień kolor…, który otwiera dialog:  
![image](https://github.com/kamilok04/Win32Playground/assets/94155059/c802576f-f2e1-47e6-8bc7-dfd5942a2a76)  

**Ważniejsze funkcje**
-	DrawTrig() – dynamicznie tworzy funkcje
-	AnimateTrig() – odpowiada za animacje robiąc zrzuty ekranu (!)

**Znane problemy**
-	Funkcja AnimateTrig() ma problemy z przewijaniem, kiedy część okna znajduje się poza ekranem. Możliwa przyczyna: część okna, która nie jest w danej chwili na ekranie, nie jest odświeżana, a nadal są z niej pobierane dane.
-	Rozmiar wektora przechowującego współrzędne punktów na wykres jest zależny od rozmiaru okna i aktualizuje się z niewielkim, ale istniejącym opóźnieniem. Zbyt szybkie zmniejszenie rozmiaru okna powoduje zakończenie programu (operacje odczyt/zapis poza zakresem wektora). Choć naprawa błędu jest prosta, uważam nowy, ergonomiczny sposób na zamknięcie okna za bardziej przydatny. 

#### Moduł *Tree*
Nie ma tu menu, jest za to ramka z oknem ustawień:
![image](https://github.com/kamilok04/Win32Playground/assets/94155059/24117719-3268-4d24-bb44-b315711f0e29)  
W ramce można zmienić tryb pracy programu na „ręczny” (tabelka wartości) lub „automatyczny” (użytkownik podaje tekst, program robi resztę)
![image](https://github.com/kamilok04/Win32Playground/assets/94155059/b5af89eb-c11c-4b9d-bf43-ccd1947286bf)  
Niezależnie od wybranego trybu pracy, po wprowadzeniu wartości i wciśnięciu przycisku OK program wygeneruje optymalne drzewo kodowania Huffmana.
*Edycja tabelki w trybie „ręcznym”* – podwójne kliknięcie w dowolnym miejscu tabeli otworzy pole tekstowe, w którym można wpisać wymarzoną wartość. 
Po wciśnięciu Enter lub otwarciu innego pola tekstowego wartość zostanie zweryfikowana i, jeżeli uznana za poprawną, wprowadzona do tabelki.
*Konflikty* – gdyby tabelka była źle znormalizowana, występowało w niej niepoprawne prawdopodobieństwo (większe niż 1 bądź ujemne), pojawi się ikonka ostrzegawcza. 
![image](https://github.com/kamilok04/Win32Playground/assets/94155059/23c1f831-bb6a-44be-899a-4dbf57e267a8)  
**Uwaga**: Program informuje o niepoprawnych parametrach drzewa, ale i tak będzie próbował wykonać polecenie w miarę możliwości. Odpowiedzialność za stan drzewa spoczywa na użytkowniku, program wydaje wszystkie potrzebne ostrzeżenia.
**Ważniejsze funkcje**
-	CreateTreeFromList() – przerabia okno z listą na (niezoptymalizowane) drzewo
-	PrepareTree() – zoptymalizuj istniejące drzewo-
-	DrawLeaf() – narysuj liść na podstawie współrzędnych
-	DrawTree() – utwórz współrzędne na podstawie optymalnego drzewa
-	AuxiliaryEditProc() – obsługuje pola tekstowe, które pojawiają się dynamicznie, a rodzic nie może przejąć komunikatu
-	VerifyAndProcessInput() – przetwarza dane wprowadzone przez użytkownika i, jeśli uzna je za poprawne, aktualizuje listę
-	HandleConflicts() – zajmuje się konfliktami w tabeli
-	ProcessAutoText() – przetwórz pole tekstowe na drzewo
**Znane problemy**
-	Przy bardzo dużych drzewach może zabraknąć ekranu i/lub część pól może być przycięta. Rozwiązaniem jest utworzenie przewijalnego podokna, jest to jeden z kierunków rozwoju projektu.
-	Z powodów optymalizacji część drzewa, która zniknie z ekranu, już na niego nie wróci. Należy wówczas ponownie wygenerować drzewo, żadne inne dane nie uległy zmianie.

#### Dialog *O programie*
![image](https://github.com/kamilok04/Win32Playground/assets/94155059/d532bd29-2cf9-4526-bcf3-3448ae9330ce)  
Przy uruchomieniu powinna odegrać się dołączona z programem ścieżka dźwiękowa. Jeżeli jej nie ma, uruchomi się dźwięk jak przy systemowym komunikacie Informacja. Nie zawiera dodatkowych funkcji.

#### DWrite.cpp
Ta część programu nie ma swojego okna, jej zadaniem jest wczytanie i przygotowanie do użycia biblioteki Direct2D. Tutaj jest użyte programowanie obiektowe.
**Ważniejsze funkcje:**
-	CreateIndependentResources() – tworzy zasoby niezależne od sprzętu;
-	CreateDependentResources() – utwórz zasoby pod konkretne urządzenie;
-	DiscardResources() – zwalnia pamięć

### Wymagania sprzętowe klienta
- System operacyjny:
  - **Windows Vista** lub wyżej (bez dodatkowych modyfikacji systemu),
### Wymagania sprzętowe kompilacji
  - *Sprawdzane były jedynie konfiguracje używające kompilatora MSVC, wersja narzędzi platformy **v142** lub wyżej*
  - Dowolny kompilator standardu C++11 i biblioteki Windows SDK wersji **10.0** lub wyżej powinny sobie poradzić z kompilacją.


## 5. Podsumowanie, wnioski 
**Uwagi**
- W całym projekcie jest używana *notacja węgierska*<sup>[1](#hungarian)</sup>, żeby zachować zgodność z konwencjami Windows.
- Typy zmiennych są pisane wielkimi literami, a podczas kompilacji te znakowe są podmieniane na swoje szerokie/wąskie odpowiedniki, w zależności od tego, czy system docelowy ma w założeniu wspierać Unicode<sup>[2](#macro-types)</sup>; znowu chodzi o konwencje.

**Podsumowanie**
Programowanie z użyciem WinAPI jest uciążliwe, jednak programy wykonane w ten sposób działają bardzo sprawnie i zużywają mniej pamięci. Samo środowisko nie ułatwia sprawy – jego część liczy sobie już dobre 40 lat; da się odczuć przeskok technologiczny w trakcie korzystania z co starszych funkcji tego samego `<Windows.h>`. Niestety, te przeskoki da się również odczuć w kodzie, ilość rzutowań niezbędna do poprawnego działania programu jest zdecydowanie wyższa niż w przypadku korzystania z bibliotek standardowych.
**Wnioski**
- Utworzenie kodu, który jest sprawny, wydajny i do tego *przenośny*, wymaga naprawdę wiele pracy - biblioteka standardowa to naprawdę duże osiągnięcie

**Kierunki rozwoju projektu**
-	Naprawienie znanych problemów;
-	Modernizacja użytych bibliotek graficznych – część używanej przeze mnie biblioteki GDI naprawdę liczy sobie 40 lat<sup>[3](#article)</sup>;
-	Zmiana mechanizmu rysowania funkcji – ten aż się prosi o problemy 
z rozwojem;
-	Dodanie wsparcia dla funkcji nie-okresowych;
-	Intensywniejsze korzystanie z tabel tekstowych – ułatwi to ewentualny przyszły proces tłumaczenia programu na inny język;
-	(daleka przyszłość) Zmiana platformy z Win32 na UWP;
---
<sup id="hungarian">[1]</sup>Nazwa zmiennej zawiera jej typ, przykłady z projektu:
- `unsigned uID;` (**u**nsigned),
- `wchar_t lpszBuffer[20];` (**l**ong **p**ointer to **s**tring terminated with **z**ero),
- `size_t cxChar;` (**c**ount of **x**-axis pixels)
więcej tutaj: [Notacja węgierska (ang.)](https://en.wikipedia.org/wiki/Hungarian_notation)

<sup id="macro_types">[2]</sup>Przykład: typ `LPCTSTR` (**l**ong **p**ointer to **c**onstant (**t**ype) **str**ing) zostanie rozbity na `LPCWSTR` (**w**ide) lub na `LPCSTR`, w zależności od tego, czy gdzieś jest zdefiniowana flaga `UNICODE`.  
<sup id="article">[3]</sup>[magazyn "Byte" z XII 1983](https://archive.org/details/byte-magazine-1983-12/page/n49/mode/2up?view=theater)
