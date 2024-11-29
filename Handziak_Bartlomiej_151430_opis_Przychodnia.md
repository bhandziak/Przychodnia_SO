# Temat 15 – Przychodnia

### Projekt składa się z procedur/programów symulujące działanie przychodni
- Dyrektor, 
- Rejestracja, 
- Lekarz,
- Pacjent

## Opis projektu

W przychodni przyjmują pacjentów dwaj lekarze POZ (pierwszego kontaktu) oraz lekarze o
specjalizacjach: kardiolog, okulista, pediatra i lekarz medycyny pracy. 
Każdy z lekarzy w danym dniu może przyjąć określoną liczbę pacjentów:
- lekarze POZ każdy X1 osób,
- kardiolog X2 osób,
- okulista X3 osób,
- pediatra X4 osób,
- lekarz medycyny pracy X5 osób.

Lekarze POZ przyjmują ok. 60% wszystkich pacjentów (wartość losowa), lekarze specjaliści każdy po ok. 10% wszystkich pacjentów
(wartość losowa). 
Do obu lekarzy POZ jest jedna wspólna kolejka pacjentów, do każdego lekarza specjalisty kolejki są indywidualne.

<u>Lekarze POZ część pacjentów (ok. 20% z X1) kierują na dodatkowe badania do wskazanych lekarzy
specjalistów</u> – pacjent zostaje zarejestrowany do specjalisty przez lekarza POZ, jeżeli dany lekarz <u>nie
ma już wolnych terminów</u> dane pacjenta (id - skierowanie do …. - wystawił) <u>powinny zostać zapisane
w raporcie dziennym</u>. Lekarze specjaliści dodatkowo mogą skierować pacjenta <u>(ok. 10%) na badanie
ambulatoryjne po wykonaniu którego pacjent wraca do danego specjalisty i wchodzi do gabinetu bez
kolejki</u> (po uprzednim wyjściu pacjenta). Jeśli w <u>chwili zamknięcia przychodni w kolejce do lekarza
czekali pacjenci</u> to te osoby zostaną przyjęte w tym dniu ale <u>nie mogą zostać skierowane na
dodatkowe badania</u>.

## Zasady działania przychodni ustalone przez Dyrektora są następujące: 

- Przychodnia jest czynna w godzinach od Tp do Tk;
- W budynku przychodni w danej chwili może się znajdować co najwyżej N pacjentów (pozostali, jeżeli są czekają przed wejściem);
- Dzieci w wieku poniżej 18 lat do przychodni przychodzą pod opieką osoby dorosłej;
- Każdy pacjent przed wizytą u lekarza musi się udać do rejestracji;
- W przychodni są 2 okienka rejestracji, zawsze działa min. 1 stanowisko;
- Jeżeli w kolejce do rejestracji stoi więcej niż K pacjentów (K>=N/2) otwiera się drugie okienko rejestracji. Drugie okienko zamyka się jeżeli liczba pacjentów w kolejce do rejestracji jest mniejsza niż N/3;
- Osoby uprawnione VIP (np. honorowy dawca krwi) do gabinetu lekarskiego wchodzą bez kolejki;
- Jeżeli zostaną wyczerpane limity przyjęć do danego lekarza, pacjenci ci nie są przyjmowani (rejestrowani);
- Jeżeli zostaną wyczerpane limity przyjęć w danym dniu do wszystkich lekarzy, pacjenci nie są wpuszczani do budynku;
- Jeśli w chwili zamknięcia przychodni w kolejce do rejestracji czekali pacjenci to te osoby nie zostaną przyjęte w tym dniu przez lekarza. Dane tych pacjentów (id - skierowanie do …. -wystawił) powinny zostać zapisane w raporcie dziennym.

### Sygnał 1
Na polecenie Dyrektora (sygnał 1) dany lekarz bada bieżącego pacjenta i kończy pracę przed zamknięciem przychodni. Dane pacjentów (id - skierowanie do …. - wystawił), którzy nie zostali przyjęci powinny zostać zapisane w raporcie dziennym.

### Sygnał 2
Na polecenie Dyrektora (sygnał 2) wszyscy pacjenci natychmiast opuszczają budynek.

## Opis testów

### Obsługa pacjentów

1. Stwórz 2 lekarzy POZ.
2. I 20 pacjentów.
3. Sprawdź, czy ok. 60% (12) pacjentów zostanie obsłużona.

### Skierowanie na dodatkowe badania

1. Stwórz 2 lekarzy POZ.
2. I 40 pacjentów.
3. Sprawdź, czy ok. 60%*0.1=6% (2) pacjentów dostanie skierowanie do specjalisty.
4. Sprawdzić raport dzienny.
5. Sprawdzenie, czy pacjent, który wrócił od specjalisty wszedł do gabinetu bez kolejki (po uprzednim wyjściu pacjenta).

### Chwila zamknięcia przychodni

1. Stwórz 2 lekarzy POZ.
2. I 40 pacjentów.
3. Ustaw czas na czas zamknięcia przychodni.
4. Sprawdź, czy czakający pacjenci zostaną przyjęte w tym dniu oraz czy nie zostaną skierowane na dodatkowe badania.

### Test godzin pracy przychodni

1. Ustaw godziny pracy przychodni na 8:00-16:00.
2. Sprawdź, czy można rejestrować pacjentów o godzinach:
- 6:00 (przed otwarciem) – pacjent powinien zostać odrzucony.
- 8:00 (otwarcie) – pacjent powinien zostać przyjęty.
- 16:00 (zamknięcie) – pacjent powinien zostać przyjęty.
- 18:00 (po zamknięciu) – pacjent powinien zostać odrzucony.

### Limit pojemności przychodni

1. Ustaw limit N = 30 pacjentów w budynku.
2. Stwórz 50 pacjentów.
3. Sprawdź, czy 30 pacjentów znajduje się w budynku i, czy 20 czeka przed wejściem.

### Dzieci i opieka nad nimi

1. Sprawdź, czy pacjent w wieku 12 lat będzie pod opieką dorosłej osoby.

### Drugie okien w rejestracji

1. Ustaw limit N = 30 pacjentów w budynku.
2. Stwórz 20 pacjentów.
3. Ponieważ 20>15, sprawdź, czy otworzy się drugie okienko.
4. Sprawdź jeśli ilość pacjentów będzie mniejsza od 10, to czy zamknie się 2 okienko.

### Pacjent VIP

1. Stwórz 20 pacjentów.
2. Stwórz pacjenta VIP.
3. Sprawdź, czy zostanie obsłużony bez kolejki.

### Limit dla lekarza

1. Ustaw limit przyjęć dla lekarza na 5 pacjentów.
2. Dodaj 7 pacjentów do kolejki.
3. Sprawdź, czy pierwszych 5 pacjentów zostaje przyjętych, a reszta (2) zostanie odrzuconych

### Limit przychodni

1. Ustaw limit przyjęć przychodni LPP=50.
2. Stwórz 60 pacjentów.
3. Sprawdź, czy pierwszych 50 pacjentów zostaje przyjętych do budynku, a reszta (10) zostanie odrzuconych

### Sygnał 1 - wczesne skończenie pracy przez lekarza

1. Stwórz 2 lekarzy POZ.
2. Stwórz 20 pacjentów.
3. Ustaw godzinę na 12:00.
4. Dyrektor wysyła sygnał 1.
5. Sprawdź, czy skończył pracę i, czy dane pacjentów którzy nie zostali przyjęci zostały zapisane w raporcie dziennym.

### Sygnał 2 - ewakuacja

1. Stwórz 2 lekarzy POZ.
2. Stwórz 20 pacjentów.
3. Ustaw godzinę na 12:00.
4. Dyrektor wysyła sygnał 2.
5. Sprawdź, czy pacjenci opuścili budynek.

## Link do repozytorium


### Autor
Bartłomiej Handziak rok 2, 
grupa laboratoryjna 2, 
151430