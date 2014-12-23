Kod powinien być stosunkowo prosty w zrozumieniu,
stąd też pozwolę sobie pominąć szczegóły implementacyjne
w tym dokumencie, a więc do rzeczy:

1) Aplikacja to pewnie zbyt duże słowo, ale szeroko pojmowany
   mechanizm komunikacyjny pomiędzy procesami serwera, komisji
   oraz raportu został zaimplementowany w jednej, spójnej konwencji językowej.
   Skutkiem tego jest angielskie nazewnictwo programów:
   
   a) server
   b) committee
   c) report

2) Dla każdego z wymienionych wyżej programów staramy się wydzielić
   spójne logicznie fragmenty i umieścić je w osobnych modułach implementacyjnych,
   co powinno pozwolić na poprawę czytelności oraz przenośność rozwiązania.
   Wyżej wspomniane moduły określamy mianem ,,serwisów'', których to 
   interfejs jest z kolei umieszczany w plikach *_service.h(funkcje `extern`).

3) Zaprojektowany mechanizm pozwala na jednoczesne działanie co najwyżej
   MAX_DEDICATED_SERVER_THREADS liczbie Workerów.

4) W fazie inicjalizacji zasobów, serwer tworzy dwa wątki, każdy odpowiedzialny
   za komunikację z klientem konkretnego rodzaju. Opisane wyżej wątki
   nazywamy ,,dispatcherami'' i to właśnie ich wątki potomne będą się
   komunikowały z klientami typu `raport` oraz `komisja`.

