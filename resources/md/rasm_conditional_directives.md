# Directives conditionnelles

RASM supporte un ensemble de directives qui permettent d'influer sur l'assemblage en vérifiant des expressions conditionnelles. D'une manière générale, lorsque des variables sont utilisées dans ces expressions, il faut impérativement qu'elles soient définies au moment de l'évaluation.

## ASSERT

```
ASSERT condition
ASSERT condition,texte,...
```

Cette directive évalue la condition et arrête l'assemblage si la condition est fausse. Un texte optionnel (usage similaire à la directive PRINT) peut-être envoyé à la console.

```
ASSERT $<#38
ASSERT data<#3FFF,'les donnees debordent de la page!'
```

## IF, IFNOT, ELSE, ELSEIF, ELSEIFNOT condition

```
IF condition  ... ELSE ...  ENDIF
IF condition  ... ELSEIF condition ...  ENDIF
IF condition  ... ELSEIF condition ...  ELSE ... ENDIF
IFNOT condition  ... ELSE ...  ENDIF
IFNOT condition  ... ELSEIF condition ...  ENDIF
IFNOT condition  ... ELSEIF condition ...  ELSE ... ENDIF
```

Définir des blocs d'assemblage conditionnel en fonction d'une expression

```
CODE_PRODUCTION=1
if CODE_PRODUCTION
   or #80
else
  and #7F
endif
out (c),a
```

## IFDEF, IFNDEF

Définir des blocs conditionnel en fonction de l'existence ou non d'une variable, d'un label, d'un alias ou d'une macro

```
IFDEF production
or #80
ENDIF
```

## UNDEF

```
UNDEF variable
```

Supprime une variable de RASM. Si la variable n'existe pas, la directive est sans effet.

## IFUSED, IFNUSED

Définir des blocs conditionnels selon qu'une variable, un label ou un alias ait été utilisé ou non dans une expression, au moment de l'appel de ces directives.

## SWITCH, CASE, DEFAULT, BREAK, ENDSWITCH

La syntaxe est similaire au switch/case du langage C, avec la particularité de pouvoir écrire plusieurs blocs CASE avec la même valeur, ce qui offre plus de souplesse au code conditionnel (mutualiser partiellement du code entre différents cas). On termine un bloc SWITCH avec la direction ENDSWITCH.

```
grouik=5
switch grouik
nop ; en dehors du périmètre d'un CASE, ce NOP ne sera pas produit
case 3
defb 'A'
case 5
defb 'B'
case 7
defb 'C'
break
case 8
defb 'D'
case 5
defb 'E'
break
default
defb 'F'
endswitch

; sortie => BCE
```

L'analyseur passera dans le case 5, continuera dans le case 7 qui fait un break, il y aura une reprise dans le deuxième case 5. La balise default sera ignorée car nous sommes passés précédemment dans au moins un case.

