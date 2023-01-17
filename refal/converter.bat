set arg=%1
refgo Preprocess+MathMode input.dot 2>error_refal0
dot2tex -ftikz -tmath "Mod_input.dot" > input.tex  2>error_dot2tex
refgo Postprocess+MathMode input.tex 2>error_refal