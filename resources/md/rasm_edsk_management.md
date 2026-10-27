# Gestion sectorielle des DSK dans RASM

 Gestion sectorielle des DSK

Afin de gérer les disquettes en mode secteur, Rasm propose un jeu de fonctionnalité en ce sens

Certaines fonctions sont à exécution immédiate comme la création, le chargement et la mise à jour d'un EDSK

Toutes les autres fonctions sont à exécution différée, afin que Rasm ait terminé son assemblage quand ces fonctions seront appelées
Fonctionnalités à exécution immédiate

```
EDSK   CREATE,'filename.dsk:side',DATA|VENDOR|UNFORMATED,nbtracks[,INTERLACED][,OVERWRITE][,TRUESIDE]
EDSK READSECT,'filename.dsk:side','location',<exactsize>
EDSK READFILE,'filename.dsk:side','filename'[,offset[,size]]
EDSK PUTFILE,'filename.dsk:side','fichier source sur host'[,'fichier destination sur dsk'][,AMSDOS|PROT|HIDDEN|BINARY|BASIC|BASIC_PROTECTED|TYPE=n|LOAD=n|EXEC=n]
EDSK GETFILE,'filename.dsk:side','fichier source sur dsk'[,'fichier destination sur host'][,NOHEADER]
EDSK  UPGRADE,'filename.dsk:side','outputfilename.dsk'
```

Fonctionnalités à exécution différée

```
EDSK  DELFILE,'filename.dsk:side','filename'
EDSK  COPYFILE,'filename.dsk','filename','filename.dsk'[,'filename'] ; dernier param en option si vous voulez renommer
EDSK  SAVEFILE,'filename.dsk','filename',<addr>,<length>[,HIDDEN,PROT,RAW,runValue]
EDSK WRITESECT,'filename.dsk:side',<addr>,<length>,'location'
EDSK    GAPFIX,'filename.dsk:side',TRACK|ALLTRACKS,<track>
EDSK       MAP,'filename.dsk'
EDSK      DROP,'filename.dsk:side','location'
EDSK     CHECK,'filename.dsk:side','location',<size>,<addr>
EDSK       ADD,'filename.dsk:side','location',<size>,...[,TRUESIDE]
EDSK    RESIZE,'filename.dsk:side','location',<size>,...
EDSK     MERGE,'filename.dsk:side','filename.dsk:side','outputfilename.dsk'
```

Comment utiliser le paramètre ``'filename.dsk:side'`` ?

Vous pouvez renseigner ce champ avec uniquement le nom du fichier, si l'image disquette contient 2 faces alors la première sera utilisée par défaut. Si vous souhaitez utiliser les faces d'un fichier image, les possibilités sont `:A`, `:B`, `:0` et `:1`

L'option `TRUESIDE` permet dans le cas de CREATE/ADD de forcer l'entête de piste et de secteur à 1 quand on opère sur la seconde face. C'est une vraie double face pour lecteur double-face. Pour utiliser Amsdos (disquette qu'on retourne dans un lecteur simple face), n'utilisez pas cette option.

# Localisation (paramètre `location`)

La localisation définit une piste ainsi que des secteurs si besoin. Les valeurs peuvent être déclarées dans n'importe quelle base (décimale, hexa, …) et vous pouvez définir plusieurs localisations en utilisant le caractère "espace" en tant que séparateur. Avec le caractère "moins", vous pouvez aussi définir des intervales.

Exemples de localisation :
`'5'`            : piste 5
`'5:#C2'`        : secteur #C2 piste 5
`'0-5'`          : pistes 0 to 5 et secteurs selon l'ordre PHYSIQUE !
`'0-5:$C2-0xC9'` : secteurs #C2 à #C9 sur les pistes 0 à 5
`'0:#C1 0:#C3'`  : secteurs #C1 et #C3 sur la piste 0

Pour l'écriture, vous pouvez utiliser le `+` avec un numéro de piste si vous ne souhaitez pas indiquer de piste de fin (usage déconseillé mais possible)

`'5+'`           : À partir de la piste 5 jusqu'à la fin de la disquette autorisé, selon l'ordre PHYSIQUE des secteurs
`'5+:0xC1:0xC9'`  : À partir de la piste 5 jusqu'à la fin de la disquette autorisé, selon l'ordre logique des secteurs DATA

# EDSK CREATE,'filename.dsk',DATA|VENDOR|UNFORMATED,nbtracks[,INTERLACED][,OVERWRITE]

Création d'une disquette au format DATA, VENDOR ou avec des pistes vides (Utilisation conjointe avec ADD), le nombre de pistes est celui par face.
On peut préciser en option le mot clef INTERLACED pour les disquettes DATA ou VENDOR.
On peut ajouter en option le mot OVERWRITE pour écraser une disquette existante avec le même nom.

# EDSK PUTFILE,'filename.dsk:side','fichier source sur host'[,'fichier destination sur dsk'][,AMSDOS|PROT|HIDDEN|BINARY|BASIC|BASIC_PROTECTED|TYPE=n|LOAD=n|EXEC=n]

La méthode la plus simple consiste à utiliser 2 ou 3 paramètres : le premier correspond à la disquette, le deuxième au fichier sur votre hôte. Le troisième paramètre, facultatif, est le nom de fichier AMSDOS sur la disquette

Si la disquette n'existe pas, elle sera créée (en mode DATA, non entrelacé)

Vous pouvez ensuite ajouter les tags facultatifs PROT ou HIDDEN pour protéger le fichier ou le masquer à la commande CAT.

Cela dit, si votre fichier ne contient pas d'en-tête AMSDOS, il sera considéré comme un fichier ASCII.

Vous pouvez créer un fichier d'en-tête BASIC/BINARY ou même un fichier BASIC_PROTECTED. Utilisez ces balises et, surtout, utilisez la balise AMSDOS !

Vous pouvez ensuite définir l'adresse de chargement ou l'adresse d'exécution avec LOAD=<expression> et EXEC=<expression>

# EDSK READFILE,'filename.dsk:side','filename',offset,size

Lire un fichier et insérer les données dans l'espace courant. L'offset et la taille sont des paramètres optionnels.

# EDSK DELFILE,'filename.dsk:side','filename'

Effacer le fichier de la disquette

# EDSK READSECT,'filename.dsk','location',<exactsize>

Lire des secteurs en mémoire. La quantité de données à lire est obligatoire. Si il n'y a pas assez de secteurs pour atteindre la quantité de données, Rasm déclenchera une erreur.

# EDSK UPGRADE,'filename.dsk','outputfilename.dsk'

Cette fonction a double usage. Elle permet de convertir un vieux DSK en EDSK ou plus simplement de copier un fichier DSK vers un autre.

# EDSK WRITESECT,'filename.dsk',<start_addr>,<length>,'location'

Enregistrer les données assemblées sur une disquette. Les données seront lues depuis la bank où est située le WRITESECT. Si la liste des pistes/secteurs fournie est insuffisante pour enregistrer le total des données, une erreur sera déclenchée.

# EDSK GAPFIX,'filename.dsk',TRACK|ALLTRACKS,<track>

Recalcule automatiquement la longueur de GAP pour que le tour de piste soit inférieur ou égal à 6250 octets

# EDSK MAP,'filename.dsk'

Affiche une cartographie des secteurs de la disquette ainsi que les informations de piste

# EDSK DROP,'filename.dsk','location'

Effacer un ou plusieurs secteurs de la disquette.

# EDSK ADD,'filename.dsk','location',<size>,…

Ajouter un ou plusieurs secteurs sur la disquette, de taille <size> correspondant à la taille FDC soit:

0 : 128
1 : 256
2 : 512
3 : 1024
4 : 2048
5 : 4096
6 : 6250 ; taille tronquée

Les tailles supérieures n'ont aucun intérêt à être gérées.

# EDSK RESIZE,'filename.dsk','location',<size>,…

Permet de redimensionner un ou plusieurs secteurs, la taille correspond à la taille FDC (voir tableau ci dessus)

# EDSK MERGE,'filename.dsk:side','filename.dsk:side','outputfilename.dsk'

Fusionner deux faces d'images simples ou double faces en un fichier double face

