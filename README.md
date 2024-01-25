# Projekt zaliczeniowy
**Programowanie I**
**X 2023 - I 2024**
**Kamil Bublij gr. 4/7**

## 1. Opis projektu
Celem projektu jest zademonstrowanie wszechstronności tak języka C++, jak i środowiska Win32, w kilku praktycznych scenariuszach.

## 2. Funkcjonalność
Program pełni kilka praktycznych funkcji, zawierając:
- generator wykresów funkcji trygonometrycznych,
- generator optymalnych drzew kodowania Huffmana,
- proste narzędzie do podawania graficznych właściwości systemu użytkownika.

## 3. Przebieg realizacji
Program jest zbudowany na zasadzie modułów - każda jego większa część ma własny plik źródłowy i osobny plik nagłówka.
### Spis plików i funkcjonalności
- `main.cpp`, `main.h` - rdzeń programu, kod odpowiedzialny za wywoływanie modułów (ich okien i dialogów); menu <s>(w tym kontekstowe)</s>
- `GraphicsDemo.cpp`, `GraphicsDemo.h` - wczytywanie parametrów systemu, tworzenie i rysowanie wielolinijkowego tekstu, mechanizm przewijania okna
-  `Trigonometry.c`, `Trigonometry.h` - funkcje interfejsu GDI, nacisk na dialogi i kontrolki (lista wybierana, pola tekstowe, suwaki), rysowanie wykresów funkcji
-  `Tree.cpp`, `Tree.h` - struktury danych (drzewa binarne), algorytmika, wielowarstwowy model rodzic-dziecko z bardziej zaawansowanym przekazywaniem danych (haki), weryfikacja danych wejściowych, funkcje GDI,
- `Resources.rc`, `resource.h` - pliki `.rc` przechowują statyczne dane (szablony okien dialogowych, tekst, obrazy, ...) aplikacji Win32. Powiązane z nimi pliki `.h` umożliwiają użycie ich w kodzie C++.

### Zastosowane algorytmy
Wszystkie podane algorytmy używają struktury `LEAF` (odtąd nazywanej *liściem*) jako podstawowej jednostki:
```cpp
// w praktyce wchar_t zostanie automatycznie podmienione na char,
// jeśli system nie wspiera Unicode
typedef struct LEAF{ 
    unsigned int ID = NULL; 
	wchar_t tSymbol[20] = {};
	wchar_t tFPValue[20] = {};
	double FPValue = NULL;
	struct LEAF* LeftChild = nullptr;
	struct LEAF* RightChild = nullptr;
} LEAF, *PLEAF;
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
### Wymagania sprzętowe klienta
- System operacyjny:
  - **Windows Vista** lub wyżej (bez dodatkowych modyfikacji systemu),
  - **Windows 2000 SP 4** lub wyżej (po zmodyfikowaniu).
  - zgodność ze starszymi wersjami nie była sprawdzana
### Wymagania sprzętowe kompilacji
  - *Sprawdzane były jedynie konfiguracje używające kompilatora MSVC, wersja narzędzi platformy **v142** lub wyżej*
  - Dowolny kompilator standardu C++11 i biblioteki Windows SDK wersji **10.0** lub wyżej powinny sobie poradzić z kompilacją.


## 5. Podsumowanie, wnioski 
**Uwagi**
- W całym projekcie jest używana *notacja węgierska*<sup>[1](#hungarian)</sup>, żeby zachować zgodność z konwencjami Windows.
- Typy zmiennych są pisane wielkimi literami, a podczas kompilacji te znakowe są podmieniane na swoje szerokie/wąskie odpowiedniki, w zależności od tego, czy system docelowy ma w założeniu wspierać Unicode<sup>[2](#macro-types)</sup>; znowu chodzi o konwencje.

<sup id="hungarian">[1]</sup>Nazwa zmiennej zawiera jej typ, przykłady z projektu:
- `unsigned uID;` (**u**nsigned),
- `wchar_t lpszBuffer[20];` (**l**ong **p**ointer to **s**tring terminated with **z**ero),
- `size_t cxChar;` (**c**ount of **x**-axis pixels)
- więcej tutaj: [Notacja węgierska (ang.)](https://en.wikipedia.org/wiki/Hungarian_notation)

<sup id="macro_types">[2]</sup>Przykład: typ `LPCTSTR` (**l**ong **p**ointer to **c**onstant (**t**ype) **str**ing) zostanie rozbity na `LPCWSTR` (**w**ide) lub na `LPCSTR`, w zależności od tego, czy gdzieś jest zdefiniowana flaga `UNICODE`.

**Podsumowanie**


**Wnioski**
- Utworzenie kodu, który jest sprawny, wydajny i do tego *przenośny*, wymaga naprawdę wiele pracy - biblioteka standardowa to naprawdę duże osiągnięcie

**Kierunki rozwoju projektu**