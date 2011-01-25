_start:
    call _prolog
    call $Main_main
    call _exit

// Prolog
_prolog:
    asf 0
    // Konstruktion Metaklassen
    // CLASS_Auto
    new     1
    .addr   VMT_$Auto
    popg    0
   
    // CLASS_LKW
    new     1
    .addr   VMT_$LKW
    popg    1

    rsf
    ret

// Exit
_exit:
    asf     0
    halt
    rsf
    ret

// VMTs
VMT_$Object:

VMT_Object:
.addr -1

VMT_$Main:
.addr $Main_main

VMT_Main:

VMT_$Auto:
.addr $Auto_errmittlePS

VMT_Auto:
.addr Auto_Auto
.addr Auto_drosseln
.addr Auto_aufmotzen

VMT_$LKW:
.addr $Auto_errmittlePS

VMT_LKW:
.addr Auto_Auto
.addr Auto_drosseln
.addr Auto_aufmotzen
.addr LKW_LKW
.addr LKW_beladen
.addr LKW_entladen
// VMTs Ende //

// Klassen //

// CLASS_Object
// nix

// Object
// nix

// CLASS_Main
$Main_main:
    //    local Auto vw;
    //    local Auto bmw;
    //    local LKW man;
    asf     3

    //  initialisierung der statischen Variable von Auto
    pushg   0
    pushc   0
    putf    0

    //  initialisierung der statischen Variable von LKW
    pushg   1
    pushc   0
    putf    0

    //    vw = new Auto(45);
    new     1
    .addr   VMT_Auto
    popl    0
    pushl   0
    dup
    pushc   45
    vmcall  2,0
    drop    2

    //      bmw = new Auto(107);
    new     1
    .addr   VMT_Auto
    popl    1
    pushl   1
    dup
    pushc   45
    vmcall  2,0
    drop    2

    //      man = new LKW(1000);
    new     3
    .addr   VMT_LKW
    popl    0
    pushl   0
    dup
    pushc   45
    vmcall  2,3
    drop    2

    //      write(Auto.produziert);
    pushg   0
    getf    0
    wrint

    rsf
    ret

// Main
// nix

// CLASS_Auto
$Auto_errmittlePS:
    // kommt noch

// Auto
Auto_Auto:
//    public Auto(Integer leistung) {
//        self.leistung = leistung;
//        Auto.autos_produziert = Auto.autos_produziert + 1;
//    }
    asf     0
    //        self.leistung = leistung;
    pushl   -3
    pushl   -2
    putf    0

    //        Auto.autos_produziert = Auto.autos_produziert + 1;
    pushg    0
    pushg    0
    getf    0
    pushc   1
    add
    putf    0
   
    rsf
    ret
   
Auto_drosseln:
    // kommt noch
Auto_aufmotzen:
    // kommt noch

// CLASS_LKW
LKW_LKW:
//      public LKW(Integer kapazitaet) {
//        super.Auto(200);
//        self.kapazitaet = kapazitaet;
//        self.ladung = 0;
//        LKW.produziert = LKW.produziert + 1;
//    }
    asf 0
    //        super.Auto(200);
    pushl   -3
    pushc   200
    call    Auto_Auto
    drop    2

    //        self.kapazitaet = kapazitaet;
    pushl   -3
    pushl   -2
    putf    2

    //        self.ladung = 0;
    pushl   -3
    pushc   0
    putf    1

    //        LKW.produziert = LKW.produziert + 1;
    pushg   1
    pushg   1
    getf    0
    pushc   1
    add
    putf    0

    rsf
    ret
LKW_beladen:
    // kommt noch
LKW_entladen:
    // kommt noch

