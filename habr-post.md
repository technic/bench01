![Что самое смешное — <br> я собирал хаскель-код через LLVM-бекенд,<br> но при этом сравнивал с GCC](https://habrastorage.org/webt/rx/wc/ce/rxwcce9fs_wmc-ovltbn0t6lq9g.jpeg)

В статье [[ссылка]](https://habr.com/ru/post/483864/) было заявлено, что производительность Haskell кода превзошла код на С++. Что сразу вызвало интерес, т.к. и то и другое может генерироваться LLVM компилятором, значит либо Наskell может давать больше хинтов компилятору, либо что-то не так с С++ реализацией.  Далее мы разберём, как череда случайностей в действиях автора привела к неправильным выводам, которые описываются таблицей ниже (под катом).
<cut/>

## Предисловие
Недавно на хабре появилась очередная [статья](https://habr.com/ru/post/489136/) от @0xd34df00d про оптимизацию хаскель кода. Сравнивается в таких случаях естественно с неоспоримым лидером в производительности -- С/C++. Затем последовал [разбор](https://habr.com/ru/post/489958/) этой статьи от @yleo о том какой асм код действительно лучше, и в чём кроется различие реализаций на разных ЯП (рекомендую к прочтению). Ещё раньше (около полутора месяцев назад), была опубликована предыдущая статья из серии "Хаскель vs С/C++", и я проделал похожий разбор, но вместо того чтобы опубликовать его на Хабр -- увы, отложил в долгий ящик. Бурные дискуссии в комментариях на этой неделе побудили меня вернуться к прошлой теме. Сегодня я его наконец-то достал тот markdown документ из ящика, стряхнул пыль, ~~дописал~~, и предоставляю его на ваше обозрение.

# Введение

Напомню, что задача была про подсчёт расстояния Левенштейна [[вики]](https://ru.wikipedia.org/wiki/%D0%A0%D0%B0%D1%81%D1%81%D1%82%D0%BE%D1%8F%D0%BD%D0%B8%D0%B5_%D0%9B%D0%B5%D0%B2%D0%B5%D0%BD%D1%88%D1%82%D0%B5%D0%B9%D0%BD%D0%B0), и вот такие результаты были показаны в оригинальной статье:

<!-- скрин есть -->
|Реализация|Отн. время|
|--|--|
|С clang 9|103%|
|С gcc 9.2|125%|
|C++ gcc 9.2|163%|
|C++ clang 9|323%|

Остановимся только на С/С++, т.к. другие бенчмарки были написаны, как заметили в комментариях, по методу "Пишем одной рукой, иногда закрывая надолго глаза".  Они были добавлены как "бонус", и в рамках одной статьи их полноценно разобрать невозможно. Тем не менее, всё равно выражаю большой респект тому человеку, который в одиночку написал реализации больше, чем на 10-ке языков.

## Что подозрительно
Во-первых, сразу бросается в глаза, что С++ версия гораздо медленнее Си, что, на самом деле, странно. Далее мы найдём, где потерялся zero-cost, а в другой части, надеюсь, покажем, как именно можно использовать мета-программирование С++, чтобы обходить Си. К тому же, на clang плюсовая версия оказалась медленнее в 3 (!) раза, хотя сишный код почти такой же по скорости как хаскель+ллвм, что ожидаемо, т.к. сlang и llvm -- это один проект.

## Череда случайностей
Если проследить, то дело было так: автор написал наивный код на плюсах и скомпилировал gcc и clang. Последний оказался в два раза медленнее, и автор его отбросил. Далее он проделал пару попыток  оптимизировать код (подробнее ниже), но gcc было абсолютно фиолетово на эти изменения. После этого автор принялся за Хаскель и написал код, который делает примерно то же самое, что и плюсовый, за исключением неких, как окажется потом, важных перестановок инструкций.

# Дьявол кроется в деталях

<!--
## Какая у вас std-либа?
Деталь номер ноль, которая не относится к действиям автора т.к. на линуксе по-умолчанию идет либа от gcc. clang libc++ Что тут  конкретно было? кстати еще -flto что то меняло у меня оО
-->

## Нюанс std::min({...})
Деталь номер один, которую заметил сам автор и множество людей в комментах, это использование std::min. 

> Мне таки удалось воспроизвести ускорение в случае C++.
> Так, если вместо `std::min({delCost, insCost, substCost})`
> написать `std::min(substCost, std::min(delCost, insCost))`, 
> то время работы под clang — уменьшается до 0.840 секунд 
> Ура, быстрее всех остальных вариантов и почти хаскель.
> (Автор оригинальной статьи - 0xd34df00d)

Смотрим на хаскель версию:
```haskell
A.unsafeWrite v1 (j + 1) $ min (substCost + substCostBase) $ 1 + min delCost insCost
```
Как ни странно, тут как раз и есть два раза вызов функции `min` от двух аргументов и в том же порядке! (надеюсь, на таком уровне я понимаю хаскель правильно). Таким образом, автор, после исправления C++ версии, сам получает, что один llvm равен второму llvm. Собственно, это я ожидал с самого начала. К сожалению, предположение, что "Наskell может давать больше хинтов компилятору" не подтвердилось. Но судьба сложилась так, что изначально автор статьи "Быстрее чем С++; медленне, чем PHP" проверил эту замену только на гцц, а этому компилятору от этого ни холодно, ни жарко. Как видно в бенчмарке ниже:

|Компилятор|Время оригнала|Время исправленного (1)|
|--|--|--|--|
|haskell/llvm|910ms|-|
|gcc 9.2|1211ms|1211ms|
|clang 9|1915ms|852ms|

В реализации stdlib от gcc я не нашел каких-то специализаций для `std::min` в случае трёх элементов, хотя это не должно быть проблемой сделать на С++. На данный момент минимум там находится путём создания массива на стеке и его обхода алгоритмом `std::min_element`. В простых случаях, как уже было замечено в комментариях, разницы нет, и компилятор умеет сам выкидывать массив на стеке и генерировать оптимальный код:
```asm
f(int, int, int):
        cmp     esi, edi
        mov     eax, edx
        cmovg   esi, edi
        cmp     esi, edx
        cmovle  eax, esi
        ret
```
<u>*Примечание:*</u> `cmov*` = conditional move (условие: `g` - greater, `le` - less equal, и т.д.).  

Но что интересно, в случае, когда вовлечены указатели, это не так, и clang, в отличии от gcc, зачем-то кладёт данные на стек (туда указывает rsp регистр):
```asm
fptr(int*, int*, int*):
        mov     eax, dword ptr [rdi]
        mov     dword ptr [rsp - 12], eax
        mov     ecx, dword ptr [rsi]
        mov     dword ptr [rsp - 8], ecx
        cmp     ecx, eax
        cmovle  eax, ecx
        mov     ecx, dword ptr [rdx]
        cmp     ecx, eax
        cmovle  eax, ecx
        ret
```
Что косвенно объясняет, почему clang более чувствителен к этому изменению в исходном коде. Также в случае без `initializer_list` компилятор генерирует оптимальный asm уже при `-O1`, а с ним нужно вовлечь больше оптимизаций (`-O2`), чтобы добиться того же asm выхлопа. Таким образом, `std::min(std::initializer_list)` не совсем зеро-кост, тут создателям стд-либы, возможно, стоит подумать над перегрузками для некоторых эвристик. 

## Вынесите s1[i]
Деталь номер два, которую я обнаружил -- это другая оптимизация из Хаскель, которую потеряли в С++.

> Да вынесите вы наконец уже `s1[i]` за рамки цикла! (я)

Опять же, по роковой случайности, гцц на неё почти по барабану, а автор-то тестировал на гцц и, соответственно, забыл внести её в итоговое сравнение. Итак, это вынос `s1[i]` за тело цикла, который присутствовал уже на нулевой итерации хаскель кода
```haskell
  let s1char = s1 `BS.index` i
  let go j | j == n = pure ()
    -- Тело цикла
```
После применения этой оптимизации к коду на С++, мы получаем результаты быстрее, чем хаскель при сборке компилятором clang. Т.е. хаскель + llvm всё же добавляет какой-то оверхед, или ему не хватает `-march=native`. Самое забавное то, что, оставив строку с `std::min` без изменений, эта версия работает быстрее, чем если применить оба изменения! Значит, компилятор как-то не очень предсказуемо переставляет инструкции во время оптимизации, и некоторые решения "волей случая" оказываются быстрее, но это мы обсудим подробнее дальше.

|Компилятор|Оригинал|Исправленный (2)|Оба исправления (3)|
|--|--|--|--|
|haskell/llvm|910ms|-|-|
|gcc 9.2|1211ms|1195ms|1195ms|
|clang 9|1915ms|742ms|831ms|

## Допилить напильником

> C++ вариант приведен для сравнения.
> Его можно оптимизировать ещё немного,
> но тогда это получится C с плюсовым main'ом, что не так интересно.
>  (Автор оригинальной статьи)

Как мы уже увидели, даже маленькие исправления могут быть абсолютно непредсказуемы в зависимости от компилятора, Поэтому, я всё же попробую чуть-чуть допилить код, потому что это не сложно: 
```cpp
  size_t lev_dist(const std::string &s1, const std::string &s2) {
    const auto m = s1.size();
    const auto n = s2.size();

    std::vector<int> v0;
    v0.resize(n + 1);
    std::iota(v0.begin(), v0.end(), 0);
    auto v1 = v0;

    for (size_t i = 0; i < m; ++i) {
      v1[0] = i + 1;
      char c1 = s1[i];
      for (size_t j = 0; j < n; ++j) {
        auto substCost = c1 == s2[j] ? v0[j] : (v0[j] + 1);
        v1[j + 1] = std::min(substCost, std::min(v0[j + 1], v1[j]) + 1);
      }
      std::swap(v0, v1);
    }
    return v0[n];
  }
```
Тут я перешёл на 32-битный `int` в векторе и чуть упростил подсчёт результата -- сначала ищем минимум, потом инкремент (что опять же уже присутствует в хаскель коде).

|Компилятор|Оригинал|Допиленный (3b)|
|--|--|--|--|
|haskell/llvm|910ms|-|
|gcc 9.2|1210ms|831ms|
|clang 9|1915ms|741ms|

Ура, теперь GCC тоже ускорился. Также я пробовал заменить счётчик `j` на указатели, что внезапно замедлило GCC. В то же время скорость clang осталась на своём максимуме.

# Осознаем результаты

## LLVM == LLVM
Во-первых, мы получили, что если написать С++ код так же как Haskell код, то результат одинаковый при использовании clang-9.  К тому же, на моём процессоре Skylake C++ версия оказывается даже быстрее. Код, который я бенчмаркал, будет находится на гитхабе<!-- (ссылка в конце)-->, и можно будет проверить данный тезис на архитектуре Haswell, которую в основном использовал автор.

<b>
Итак, приходим к выводу, что вместо сравнений языков, по факту проводилось сравнение компилятора GCC и LLVM.
</b>

В оригинальной статье было детально разобрано, как на хаскеле написать код, заточенный под llvm, и обойтись без ffi, за что автору спасибо.

## GCC vs CLANG

Во-вторых, до этого момента сложилось впечатление, что старичок gcc уже ни на что не годится. Поэтому, ставьте свою генту пересобираться clang-ом и читайте дальше.

Отдельно отмечу, что в Си версии исходного кода, предоставленной автором, была директива компилятора, которая выбирает лучший код в зависимости от компилятора (`#if !defined(__GNUC__) || defined(__llvm__)`), что объясняет относительную разницу между Си реализациями и С++ реализациями, и соответственно, делать выводы о соотношении Си и С++ по приведённой автором таблице нельзя.

> clang не осиливает (либо не пытается) убрать ветвления. (Голос из зала)

Попробуем понять, чем вызвана разница между GCC и LLVM. Для этого посмотрим, что там наворотил компилятор в asm. С gcc все более-менее ясно: один внутренний цикл, который на основе команд cmov* делает min (аналогично тому, что мы видели листинге выше). Я беру версию (3), это та, что с двумя исправлениями, и С++ выглядит так:
```cpp
      for (size_t j = 0; j < n; ++j) {
        auto delCost = v0[j + 1] + 1;
        auto insCost = v1[j] + 1;
        auto substCost = c1 == s2[j] ? v0[j] : (v0[j] + 1);
        v1[j + 1] = std::min(substCost, std::min(delCost, insCost));
      }
```
Ассемблер, который я ради не сильно знакомых с ним читателей решил прокомментировать, получается таким:
```
.L42:
        inc     rcx  // j++
        mov     rdi, QWORD PTR [r12+rcx*8]  // загрузить v0[j+1]
        xor     edx, edx  // обнулить %edx
        cmp     r10b, BYTE PTR [r11-1+rcx]  // c1 == s2[j]
        setne   dl  // результат в последнем байте %rdx
        lea     r9, [rdi+1]  // стало v0[j+1] + 1
        add     rdx, QWORD PTR [r12-8+rcx*8]  // добавить v0[j]
        lea     rsi, [rax+1]  // %rax это v1[j]
        cmp     rdi, rax  // сравнить v0[j+1] и v1[j] до += 1
        mov     rax, r9
        cmovg   rax, rsi  // на основе сравнения выбрать результат после += 1
        cmp     rax, rdx  // меньшее %rax, %rdx
        cmovg   rax, rdx
        mov     QWORD PTR [r8+rcx*8], rax  // v1[j+1] = ...
        cmp     rbx, rcx  // loop
        jne     .L42
```
Тут компилятор сам сделал оптимизацию, которая упоминалась в оригинальной статье -- вместо загрузки `v1[j]` на каждой итерации мы передаем его через `%rax`.

Что же касается LLVM, то тут какая-то лапша из переходов, которую полностью приводить не буду. Отмечу лишь для примера, что во одном из кусков во внутреннем цикле имеется конструкция, частично похожая на предыдущую:
```
.LBB1_40:                               #   in Loop: Header=BB1_36 Depth=2
        mov     qword ptr [r14 + 8*rsi + 8], rax
        mov     rdx, qword ptr [rbx + 8*rsi + 16]
        inc     rdx
        inc     rax
        xor     ebp, ebp
        cmp     cl, byte ptr [r13 + rsi + 1]
        setne   bpl
        add     rbp, qword ptr [rbx + 8*rsi + 8]
        cmp     rax, rdx
        jg      .LBB1_41
        lea     rdi, [rsi + 2]
        cmp     rax, rbp
        jle     .LBB1_44
        jmp     .LBB1_43
```
<u>_Примечание:_</u> `jmp, j*` = jump (условие: `jg` - greater, `jle` - less equal, и т.д.).

Тоже загружаем данные из `v0[j+1]`, `v0[j]`, делаем `cmp` для `s1[i]`, но потом у нас идёт набор из cmp + jump во всех вариациях. Оставшуюся лапшу так детально комментировать не буду, но вполне ожидаемо, что на однотипных данных (а это то, что делал автор) [бранч предиктор](https://ru.wikipedia.org/wiki/%D0%9F%D1%80%D0%B5%D0%B4%D1%81%D0%BA%D0%B0%D0%B7%D0%B0%D1%82%D0%B5%D0%BB%D1%8C_%D0%BF%D0%B5%D1%80%D0%B5%D1%85%D0%BE%D0%B4%D0%BE%D0%B2) рулит, и такой код работает быстрее, как заметили в комментариях. Давайте попробуем на других данных -- двух случайных строках.

|Компилятор|str a - str a, str a - str b|random-random x2|
|--|--|--|
|gcc 9.2|1190 ms|1190 ms|
|clang 9|837 ms|1662 ms|

Как и ожидалось, в GCC не меняется результат ни на одну миллисекунду, а LLVM замедляется в 2 (!) раза, потому что бранч предиктор больше не работает.

<b>
Итак, приходим к основному тезису статьи. По факту были сравнены две реализации алгоритмов: одна основана на условных переходах (jump), другая -- на операциях условного копирования (cmov).
Одна реализация работает лучше на однотипных данных, другая -- на случайных.
</b>

Естественно, компилятор не может знать заранее, какие данные будут у программы в реальной жизни. Для того, чтобы решить эту задачу, существует [PGO](https://ru.wikipedia.org/wiki/Profile-guided_optimization) (Кстати, тут языки с JIT могут заиграть новыми красками). Я проверил это в нашем случае и получил, что GCC после PGO выдает результат наравне с самой быстрой версией clang. Какие данные ближе к реальной задаче -- это предмет отдельной дискуссии, которую мы оставим для последующих изысканий. Мне кажется, что хоть мы и будем в реальности сравнивать близкие строки, выбор в алгоритме между удалить/заменить/вставить всё же будет случайный, а ветка, когда не надо делать ничего, может быть обработана отдельной эвристикой.

# Выводы
- Бенчмарки порождают холивары, а холивары -- новые бенчмарки
- Никакой дискриминации нет, LLVM генерирует хороший код для всех
- Мало того, что GCC и LLVM дают разную скорость в зависимости от задачи, так еще и в зависимости от входных данных
- Бенчмарки без четкого технического задания и полноценного набора входных тестовых данных не имеют смысла
- Автору надо прикручивать обратно ffi. На самом деле нет, т.к. у него есть другой алгоритм, о чём, надеюсь, узнаем в других сериях
- Не спешите бежать на новый язык или компилятор на основе бенчмарков в интернете

Надеюсь, в следующей части будут рассмотрены детально эти два подхода и разобраны их плюсы и минусы в более реальных ситуациях, а так же предложены способы ускорения этого алгоритма.

## Где я мог обмануть
Для полной корректности выводов надо было провести ещё и следующий эксперимент: Убирать оптимизации из Хаскель версии и проверять, стало ли оно медленнее, тогда можно было бы более полно проверить тезис об "умности" компиляторов и, в частности, о влиянии алиасинга. Но эту задачку я оставлю любителям ФП или Rust (Блин, я же сам в числе последних).

## P.S. Альтернативное решение
> Первый способ решить задачу -- это проверить, решил ли её уже кто-то другой 
> (Мой препод по матану)

Напомню, что задача -- это поиск редакционного расстояния, т.е. минимального числа вставок, удалений и замен символов, которые надо произвести, чтобы из строки s1 получить строку s2. Статья об этом уже была [на хабре](https://habr.com/ru/post/117063/).  В данной заметке мы рассмотрели способы оптимальной реализации алгоритма Вагнера-Фишера, который требует O(n*m) времени (два вложенных for). По ссылке выше есть ещё алгоритм Хиршберга, но он тоже работает за квадратичное время. Хотя всё же можно ускорить алгоритм, если нас не интересуют расстояния больше некоторого наперёд заданного k. Так же, есть трюк который должен позволить сделать векторизацию. Об этом писал автор обсуждаемой здесь статьи, но это уже тема для другой заметки.

*Спасибо @LinearLeopard за исправление ошибки в этом абзаце.*

# Приложение
### Методика бенчей
- Флаги компилятора: `-O3 -march=native -std=gnu++17`.
- Процессор: Intel i5-8250U (да, ноут)
- ОС: Ubuntu 19.04 / Linux 5.0.0
- Первый прогон для разгона турбо-буст, далее берём минимум из пяти подряд. Отклонения в рамках 1-2%.
- Между запусками разных реализаций 1с прохлаждаемся (да, ноут)

### Добавлено: скрипты для ленивых
Можете запустить всё то же самое на своем железе и сообщить общественности результат: [ссылка на гитхаб](https://github.com/technic/bench01).

### Добавлено: Результаты без -march=native
По заявкам в комментариях, решил проверить влияние этого флага. 
|Флаги|-O3 -march=native|-O3 -march=native|-O3|-O3|
|--|--|--|--|--|
|Компилятор|Оригинал|Допиленный (3b)|Оригинал|Допиленный (3b)|
|haskell/llvm|-|-|910ms|-|
|gcc 9.2|1210ms|831ms|1191ms|791ms|
|clang 9|1915ms|741ms|1924ms|834ms|

