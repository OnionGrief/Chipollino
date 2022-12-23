# Chipollino
На программе вход даётся файл с коммандами (по умолчанию *test.txt*). Комманды записываются без скобок и запятых

Пример входного файла:
```
N1 = Glushkov {((ab)*|a)*} !!
N2 = Glushkov {(a|(ab)*)*}
N3 = Annote.Determinize N1
Equal N1 N2
Test {ab(ab|a)*ababa} {((ab)*a)*} 1
SemDet N1
SemDet N2
```

Для создания отчета необходимо установить:
* [Graphviz](https://graphviz.org/download/)
* [Latex](https://www.latex-project.org/get/)

Доступный функционал указан [тут](https://github.com/StarikTenger/Chipollino/wiki/%D0%94%D0%BE%D1%81%D1%82%D1%83%D0%BF%D0%BD%D1%8B%D0%B5-%D1%84%D1%83%D0%BD%D0%BA%D1%86%D0%B8%D0%B8)
