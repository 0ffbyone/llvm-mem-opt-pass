llvm pass для оптимизации вызовов функций-аллокаторов.
Этот pass написан с использованием нового PassMangera'а llvm.

Для компиляции `sh build.sh`. В директории 'tests/' хранятся различные программы,
которые использовались для проверки pass'а. Чтобы запустить pass для какой-то
программы из 'tests/' нужно `sh pass.sh {file-name}`


На данный момент пасс проходит и говорит можно ли оптимизировать данную
программу или нет.

Несколько слов про реализацию, пасс считает функции аллокаторы - те функции,
которые имеют аттриубут `allocsize`. Я почти уверен что это не все функции,
которые умеют аллоцировать память. В примере котором вы дали мне при
встрече, функция `my_malloc` не имеет такого аттрибута, но имеет аттрибут
`allockind("alloc,uninitialized")`, но по какой-то причине стандартный
`malloc` не имет аттрибута `allockind`. В крайнем случае можно изменить пасс
чтобы он принимал в качестве функции-аллокатора такие функции, которые
будут иметь аттрибут `allocsize` или `allockind`.

Также у этого пасса есть и другие ограничения. Например он
работает с одной веткой вычислений и одной функцией аллокатором.
Это можно починить, но кода для этого потребуется куда больше.


В качестве примера я предлагаю посмотреть на две функции `/tests/opt.c`
и `/tests/no-opt.c`. Первую мы оптимизировать можем, тк, только в
в низковероятной ветви вычислений мы используем выделенную память.
Вторую же оптимизировать уже нельзя тк в высоковероятной ветви вычислений
мы используем аллоцированную память и аллоцировать ее внутри особого смысла нет.

Мне еще необходимо написать сам Transformation Pass для изменения кода.


============================== Вторая итерация ==============================

Добавлена возможность работы с несколькими ветками `SwitchInst`,
баг при котором, обращение к аллоцированной памяти происходило
не только в оптимизируемой ветке, но при этом пасс выдавал возможность оптимизации -
устранен. Единственное замечание про веса условий, про него я писал в чате.
Условие при котором пасс работает, это обычные веса которые не будут одновременно
использовать `LIKELY(X)` и `UNLIKELY(X)`, в этом случае llvm крайне странно выдает веса
условиям.
Оптимизация сейчас работает только с первой аллокацией памяти, она проходит по всем
веткам, находит все `UNLIKELY` ветки по весам, проверяет, что толькой в одной из
них мы обращаемся к выделенной памяти, проверяет, что нигде кроме как
в этой ветке мы не используем эту память и в этом случаи перемещает llvm `CallInst`
аллокации памяти в тот `UNLIKELY` `BasicBlock` в котором мы и обращаемся к нашей памяти.

Проверить работу можно все также с помощью `pass.sh`, получившиеся IR появляются
в `tests/IROptimized`, туда направляются все IR, и оптимизированные и не измененные,
в стандартный вывод выводится фраза, был ли оптимизирован IR или нет,
если он был оптимизирован, то IR с одноименным названием в `tests/IR` в `tests/IROptimized`
будет изменен, если же IR не был оптимизирован, то в этих двух местах IR будет совпадать.

В качестве примера оптимизация `main-test-two.ll`

Было
```llvm
define dso_local i32 @foo(i32 noundef %0) local_unnamed_addr #0 {
  %2 = tail call noalias dereferenceable_or_null(1024) ptr @malloc(i64 noundef 1024) #3
  %3 = icmp eq i32 %0, 42
  br i1 %3, label %4, label %5, !prof !5

4:                                                ; preds = %1
  tail call void @bar(ptr noundef %2) #4
  br label %5

5:                                                ; preds = %1, %4
  %6 = phi i32 [ 1, %4 ], [ 0, %1 ]
  ret i32 %6
}

```

Стало
```llvm
define dso_local i32 @foo(i32 noundef %0) local_unnamed_addr #0 {
  %2 = icmp eq i32 %0, 42
  br i1 %2, label %3, label %5, !prof !5

3:                                                ; preds = %1
  %4 = tail call noalias dereferenceable_or_null(1024) ptr @malloc(i64 noundef 1024) #3
  tail call void @bar(ptr noundef %4) #4
  br label %5

5:                                                ; preds = %3, %1
  %6 = phi i32 [ 1, %3 ], [ 0, %1 ]
  ret i32 %6
}

```



