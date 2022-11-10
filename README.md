# Chipollino
На программе вход даётся файл с коммандами (по умолчанию *test.txt*). Комманды записываются без скобок и запятых

Пример входного файла:
```
N1 = Glushkov ((ab)*|a)* !!
N2 = Glushkov (a|(ab)*)*
N3 = Annote.Determinize N1
Equal N1 N2
Test ab(ab|a)*ababa ((ab)*a)* 1
SemDet N1
SemDet N2
```

Для создания отчета необходимо установить:
* [Graphviz](https://graphviz.org/download/)
* [Latex](https://www.latex-project.org/get/)
