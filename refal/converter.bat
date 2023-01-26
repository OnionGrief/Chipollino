set arg=%1
IF EXIST "Aux_"%arg%".data" DEL "Aux_"%arg%".data" 
IF EXIST "Meta_"%arg%".data" DEL "Meta_"%arg%".data" 
IF EXIST "L_"%arg%".tex" DEL "L_"%arg%".tex"  
refgo Preprocess+MathMode %arg%".dot" 2>error_refal0
dot2tex -ftikz -tmath "Mod_"%arg%".dot" > %arg%".tex"  2>error_dot2tex
refgo Postprocess+MathMode %arg%".tex" 2>error_refal