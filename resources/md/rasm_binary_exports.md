## SAVE

La directive SAVE permet de nombreuses exportations différentes

```
SAVE 'monfichier.bin',debut,taille ; enregistrer le code binaire produit de début à fin

SAVE 'monfichier.bin',debut,taille,AMSDOS ; enregistrer le code binaire produit de début à fin avec un entête AMSDOS (voir la directive RUN pour spécifier une adresse d'exécution)

SAVE 'monfichier.bin',debut,taille,HOBETA ; entête HOBETA pour ZX Spectrum

SAVE 'monfichier.bin',debut,taille,TAPE,'monfichierK7.cdt' ; enregistrer un binaire K7 au format CDT
```

### Enregistrements à l'intérieur d'une image disquette EDSK

```
SAVE 'monfi.bin',debut,taille,DSK,'madisquette.dsk'
```

Si le fichier DSK n'existe pas, il sera créé (au format DATA), si le fichier existe, il sera modifié (voir l'option de ligne de commande -eo pour écraser un fichier déjà présent sur la disquette)

## Limitations

N'oubliez pas que l'AMSDOS tolère mal les noms de fichiers sans extension. Si votre nom de fichier dépasse 8 caractères, il sera tronqué.

La mise à jour des disquettes n'est pas "physique", tous les fichiers sont lus en mémoire, ceux mis à jour sont remplacés en mémoire, puis une nouvelle disquette est écrite avec tous les fichiers présents en mémoire. Pour le moment les USER ne sont pas supportés. Autre conséquence, la mise à jour ne permet pas de travailler avec des formats spéciaux ou un système de TrackLoad. Vous pouvez toujours jeter à oeil à "edsktool" pour le faire.

## Transformation d'un fichier binaire simple en fichier AMSDOS

```
org #100
run #100 ; or anything else, it's optionnal but if you want another entry point than loading address, it's here
incbin 'binary_without_header.bin'
save 'binary_with_header',#100,$-#100,AMSDOS
```

## Création d'une cartouche à partir de fichiers ROM

```
buildcpr
bank 0 : incbin 'binary_bank0.bin'
bank 1 : incbin 'binary_bank1.bin'
bank 2 : incbin 'binary_bank2.bin'
bank 3 : incbin 'binary_bank3.bin'
; ...
```

