#include "PluginMenuStrings.h"

namespace Plugins
{

const MenuStrings& menuStrings(int language)
{
    static const MenuStrings table[134] = {
        { "PAUSE",          "Continue",          "Skip",               "Withdraw",            "Yes",         "No",        "Are you sure?"          }, // 0  English
        { "ポーズ",          "つづける",           "スキップ",             "たいきゃく",           "はい",         "いいえ",      "ほんとうに？"             }, // 1  Japanese
        { "PAUSE",          "Continuer",         "Passer",             "Retraite",            "Oui",         "Non",       "Êtes-vous sûr ?"        }, // 2  French
        { "PAUSE",          "Fortfahren",        "Überspringen",       "Rückzug",             "Ja",          "Nein",      "Bist du sicher?"        }, // 3  German
        { "PAUSA",          "Continua",          "Salta",              "Ritirati",            "Sì",          "No",        "Sei sicuro?"            }, // 4  Italian
        { "PAUSA",          "Continuar",         "Saltar",             "Retirarse",           "Sí",          "No",        "¿Estás seguro?"         }, // 5  Spanish
        { "暂停",            "继续",               "跳过",                "撤退",                "是",          "否",        "你确定吗？"              }, // 6  Chinese
        { "POUSE",          "Gaan voort",        "Oorslaan",           "Onttrek",             "Ja",          "Nee",       "Is jy seker?"           }, // 7  Afrikaans
        { "ማቆም",           "ቀጥል",              "ዝለል",                "ማፈግፈግ",              "አዎ",          "አይ",        "እርግጠኛ ነህ?"             }, // 8  Amharic
        { "إيقاف مؤقت",     "متابعة",            "تخطي",               "انسحاب",              "نعم",         "لا",        "هل أنت متأكد؟"          }, // 9  Arabic
        { "PAUSE",          "Continue",          "Skip",               "Withdraw",            "Yes",         "No",        "Are you sure?"          }, // 10 Mapudungun
        { "إيقاف مؤقت",     "متابعة",            "تخطي",               "انسحاب",              "نعم",         "لا",        "هل أنت متأكد؟"          }, // 11 Moroccan Arabic
        { "বিৰতি",            "চলাই যাওক",            "এৰি যাওক",             "উভতি যোৱা",          "হয়",         "নহয়",       "আপুনি নিশ্চিত নে?"        }, // 12 Assamese
        { "PAUSE",          "Davam et",          "Keç",                "Geri çəkil",          "Bəli",        "Xeyr",      "Əminsiniz?"             }, // 13 Azerbaijani
        { "ПАУЗА",          "Давам",             "Атлап үтеү",         "Сигеү",               "Эйе",         "Юҡ",        "Ышанаһыңмы?"            }, // 14 Bashkir
        { "ПАЎЗА",          "Працягнуць",        "Прапусціць",         "Адступіць",           "Так",         "Не",        "Вы ўпэўнены?"           }, // 15 Belarusian
        { "ПАУЗА",          "Продължи",          "Пропусни",           "Оттегляне",           "Да",          "Не",        "Сигурни ли сте?"        }, // 16 Bulgarian
        { "বিরতি",            "চালিয়ে যান",            "এড়িয়ে যান",            "প্রত্যাহার",           "হ্যাঁ",        "না",        "আপনি কি নিশ্চিত?"        }, // 17 Bengali
        { "མཚམས",          "མུ་མཐུད",              "མཆོང",               "ཕྱིར་འཐེན",            "ཡིན",         "མིན",       "ངེས་གཏན་ཡིན་ནམ།"        }, // 18 Tibetan
        { "PAUSE",          "Kenderc'hel",       "Lammat",             "Distreiñ",            "Ya",          "Ket",       "Sur oc'h?"              }, // 19 Breton
        { "PAUZA",          "Nastavi",           "Preskoči",           "Povlačenje",          "Da",          "Ne",        "Jeste li sigurni?"      }, // 20 Bosnian
        { "PAUSA",          "Continua",          "Salta",              "Retirar-se",          "Sí",          "No",        "N'estàs segur?"         }, // 21 Catalan
        { "وەستان",         "بەردەوامبە",        "بازبدە",             "کشانەوە",             "بەڵێ",        "نەخێر",     "دڵنیایت؟"               }, // 22 Central Kurdish
        { "PAUSA",          "Cuntinua",          "Salta",              "Ritirassi",           "Iè",          "Nò",        "Sì sicuru?"             }, // 23 Corsican
        { "PAUZA",          "Pokračovat",        "Přeskočit",          "Ustoupit",            "Ano",         "Ne",        "Jste si jisti?"         }, // 24 Czech
        { "SAIB",           "Parhau",            "Neidio",             "Encilio",             "Iawn",        "Na",        "Ydych chi'n siŵr?"      }, // 25 Welsh
        { "PAUSE",          "Fortsæt",           "Spring over",        "Træk dig",            "Ja",          "Nej",       "Er du sikker?"          }, // 26 Danish
        { "PAUS",           "Fortsäize",         "Iwwersprangen",      "Wustupiś",            "Jo",          "Ně",        "Sy sy wěsty?"           }, // 27 Lower Sorbian
        { "ވަކިކުރުން",           "ކުރިއަށް",               "ދޫކުރުން",              "ފަހަތަށްދިޔުން",          "ހޫނ",         "ނޫން",      "ޔަގީންތަ؟"              }, // 28 Divehi
        { "ΠΑΥΣΗ",          "Συνέχεια",          "Παράλειψη",          "Απόσυρση",            "Ναι",         "Όχι",       "Είστε σίγουροι;"        }, // 29 Greek
        { "PAUS",           "Jätka",             "Jäta vahele",        "Taandu",              "Jah",         "Ei",        "Kas oled kindel?"       }, // 30 Estonian
        { "PAUS",           "Jätka",             "Jäta vahele",        "Erretiratu",          "Bai",         "Ez",        "Ziur zaude?"            }, // 31 Basque
        { "توقف",           "ادامه",             "پرش",                "عقب‌نشینی",           "بله",         "خیر",       "مطمئنی؟"                }, // 32 Persian
        { "TAUKO",          "Jatka",             "Ohita",              "Vetäydy",             "Kyllä",       "Ei",        "Oletko varma?"          }, // 33 Finnish
        { "PAUSE",          "Magpatuloy",        "Laktawan",           "Umatras",             "Oo",          "Hindi",     "Sigurado ka ba?"        }, // 34 Filipino
        { "PAUSE",          "Halda fram",        "Sleppa",             "Trekk tilbake",       "Ja",          "Nei",       "Ert tú vísur?"          }, // 35 Faroese
        { "PAUSE",          "Fortsätt",          "Hoppa över",         "Weromlûke",           "Ja",          "Nee",       "Bisto wis?"             }, // 36 Frisian
        { "PAUSE",          "Lean ort",          "Scipeáil",           "Cúlú",                "Tá",          "Níl",       "An bhfuil tú cinnte?"   }, // 37 Irish
        { "PAUSE",          "Lean ort",          "Leum thairis",       "Tarraing air ais",    "Tha",         "Chan eil",  "A bheil thu cinnteach?" }, // 38 Scottish Gaelic
        { "PAUSE",          "Continue",          "Skip",               "Withdraw",            "Yes",         "No",        "Are you sure?"          }, // 39 Gilbertese
        { "PAUSA",          "Continuar",         "Saltar",             "Retirarse",           "Si",          "Non",       "Estás seguro?"          }, // 40 Galician
        { "PAUSE",          "Weiter",            "Überspringen",       "Zrugzieh",            "Ja",          "Nei",       "Bisch sicher?"          }, // 41 Swiss German
        { "વિરામ",           "ચાલુ રાખો",           "છોડી દો",             "પાછા ખેંચો",          "હા",          "ના",        "શું તમે ખાતરી ધરાવો છો?"   }, // 42 Gujarati
        { "PAUSE",          "Ci gaba",           "Tsallake",           "Janyewa",             "Ee",          "A'a",       "Ka tabbata?"            }, // 43 Hausa
        { "השהיה",          "המשך",              "דלג",                "נסיגה",               "כן",          "לא",        "האם אתה בטוח?"          }, // 44 Hebrew
        { "विराम",          "जारी रखें",           "छोड़ें",               "वापसी",               "हाँ",         "नहीं",       "क्या आप निश्चित हैं?"     }, // 45 Hindi
        { "PAUZA",          "Nastavi",           "Preskoči",           "Povlačenje",          "Da",          "Ne",        "Jeste li sigurni?"      }, // 46 Croatian
        { "PAUS",           "Pokračować",        "Přeskočić",          "Wustupić",            "Jo",          "Ně",        "Sy sej wěsty?"          }, // 47 Upper Sorbian
        { "SZÜNET",         "Folytatás",         "Kihagyás",           "Visszavonulás",       "Igen",        "Nem",       "Biztos vagy benne?"     }, // 48 Hungarian
        { "ԴԱԴԱՐ",          "Շարունակել",        "Բաց թողնել",         "Նահանջել",            "Այո",         "Ոչ",        "Վստա՞հ եք"              }, // 49 Armenian
        { "JEDA",           "Lanjutkan",         "Lewati",             "Mundur",              "Ya",          "Tidak",     "Anda yakin?"            }, // 50 Indonesian
        { "PAUSE",          "Gaa n'ihu",         "Mafee",              "Wezụga",              "Ee",          "Mba",       "Ị ji n'aka?"            }, // 51 Igbo
        { "ꀧꅇ",             "ꆏꌠ",                "ꀋꆏ",                 "Withdraw",            "Yes",         "No",        "Are you sure?"          }, // 52 Yi
        { "HLÉ",            "Halda áfram",       "Sleppa",             "Hörfa",               "Já",          "Nei",       "Ertu viss?"             }, // 53 Icelandic
        { "ᐃᓱᒪᖅ",          "ᑲᔪᓯᓗᑎᑦ",             "ᐃᓕᕋᐃᑦ",              "Withdraw",            "Yes",         "No",        "Are you sure?"          }, // 54 Inuktitut
        { "პაუზა",          "გაგრძელება",        "გამოტოვება",         "გაყვანა",             "დიახ",        "არა",       "დარწმუნებული ხართ?"     }, // 55 Georgian
        { "ПАУЗА",          "Жалғастыру",        "Өткізу",             "Шегіну",              "Иә",          "Жоқ",       "Сенімдісіз бе?"         }, // 56 Kazakh
        { "PAUSE",          "Nangma",            "Skip",               "Withdraw",            "Yes",         "No",        "Are you sure?"          }, // 57 Greenlandic
        { "ផ្អាក",            "បន្ត",                 "រំលង",               "ដកថយ",               "បាទ",         "ទេ",        "តើអ្នកប្រាកដទេ?"          }, // 58 Khmer
        { "ವಿರಾಮ",           "ಮುಂದುವರಿಸಿ",           "ಬಿಟ್ಟುಹೋಗಿ",            "ಹಿಂತೆಗೆದುಕೊಳ್ಳಿ",        "ಹೌದು",        "ಇಲ್ಲ",       "ನಿಮಗೆ ಖಚಿತವಿದೆಯೇ?"       }, // 59 Kannada
        { "일시정지",        "계속",               "건너뛰기",              "철수",                "예",          "아니요",      "확실합니까?"              }, // 60 Korean
        { "विराम",          "जारी ठेवा",          "वगळा",               "माघार",               "व्हय",        "ना",        "तुमका खात्री आसा?"       }, // 61 Konkani
        { "وەستان",         "بەردەوامبە",        "بازبدە",             "کشانەوە",             "بەڵێ",        "نەخێر",     "دڵنیایت؟"               }, // 62 Kurdish
        { "ՏԱԴԱՐ",          "Շարունակել",        "Բաց թողնել",         "Նահանջել",            "Ооба",        "Жок",       "Ишенимдүүсүзбү?"        }, // 63 Kyrgyz
        { "PAUS",           "Weider",            "Iwwersprangen",      "Zréckzéien",          "Jo",          "Neen",      "Bass du sécher?"        }, // 64 Luxembourgish
        { "ຢຸດ",            "ສືບຕໍ່",            "ຂ້າມ",              "ຖອນຕົວ",              "ແມ່ນ",        "ບໍ່",       "ທ່ານແນ່ໃຈບໍ?"           }, // 65 Lao
        { "PAUZĖ",          "Tęsti",             "Praleisti",          "Trauktis",            "Taip",        "Ne",        "Ar tikrai?"             }, // 66 Lithuanian
        { "PAUZE",          "Turpināt",          "Izlaist",            "Atkāpties",           "Jā",          "Nē",        "Vai esi pārliecināts?"  }, // 67 Latvian
        { "Oki",            "Haere tonu",        "Tīpoka",             "Hoki",                "Āe",          "Kāo",       "Kei te tino mōhio koe?" }, // 68 Māori
        { "ПАУЗА",          "Продолжи",          "Прескокни",          "Повлекување",         "Да",          "Не",        "Дали сте сигурни?"      }, // 69 Macedonian
        { "വിരാമം",           "തുടരുക",             "ഒഴിവാക്കുക",           "പിന്മാറുക",           "അതെ",         "ഇല്ല",       "നിങ്ങൾക്ക് ഉറപ്പാണോ?"     }, // 70 Malayalam
        { "ЗОГСООХ",        "Үргэлжлүүлэх",      "Алгасах",            "Ухрах",               "Тийм",        "Үгүй",      "Итгэлтэй байна уу?"     }, // 71 Mongolian
        { "PAUSE",          "Continue",          "Skip",               "Withdraw",            "Yes",         "No",        "Are you sure?"          }, // 72 Mohawk
        { "विराम",          "सुरू ठेवा",            "वगळा",               "माघार",               "होय",         "नाही",      "तुम्हाला खात्री आहे का?"   }, // 73 Marathi
        { "PAUSE",          "Fortsæt",           "Spring over",        "Berundur",            "Ya",          "Tidak",     "Anda pasti?"            }, // 74 Malay
        { "PAUSE",          "Kompli",            "Aqbeż",              "Irtira",              "Iva",         "Le",        "Int ċert?"              }, // 75 Maltese
        { "ခဏရပ်",          "ဆက်ရန်",            "ကျော်ရန်",             "ထွက်ခွာ",             "ဟုတ်ကဲ့",      "မဟုတ်ဘူး",   "သေချာပါသလား?"          }, // 76 Burmese
        { "PAUSE",          "Fortsett",          "Hopp over",          "Trekk deg",           "Ja",          "Nei",       "Er du sikker?"          }, // 77 Norwegian (Bokmål)
        { "विराम",          "जारी राख्नुहोस्",      "छोड्नुहोस्",           "फिर्ता",              "हो",          "होइन",      "के तपाईं निश्चित हुनुहुन्छ?" }, // 78 Nepali
        { "PAUZE",          "Doorgaan",          "Overslaan",          "Terugtrekken",        "Ja",          "Nee",       "Weet je het zeker?"     }, // 79 Dutch
        { "PAUSE",          "Hald fram",         "Hopp over",          "Trekk deg",           "Ja",          "Nei",       "Er du sikker?"          }, // 80 Norwegian (Nynorsk)
        { "PAUSE",          "Fortsett",          "Hopp over",          "Trekk deg",           "Ja",          "Nei",       "Er du sikker?"          }, // 81 Norwegian
        { "PAUSA",          "Continuar",         "Saltar",             "Retirar-se",          "Òc",          "Non",       "Sès segur?"             }, // 82 Occitan
        { "ବିରତି",           "ଜାରି ରଖନ୍ତୁ",          "ଛାଡ଼ନ୍ତୁ",             "ପଛକୁ ଫେରନ୍ତୁ",         "ହଁ",          "ନାହିଁ",      "ଆପଣ ନିଶ୍ଚିତ କି?"          }, // 83 Odia
        { "PAUSE",          "Kontinuá",          "Skipe",              "Retirá",              "Sí",          "No",        "Bo ta sigur?"           }, // 84 Papiamento
        { "ਵਿਰਾਮ",           "ਜਾਰੀ ਰੱਖੋ",             "ਛੱਡੋ",                "ਵਾਪਸੀ",               "ਹਾਂ",         "ਨਹੀਂ",       "ਕੀ ਤੁਹਾਨੂੰ ਯਕੀਨ ਹੈ?"       }, // 85 Punjabi
        { "PAUZA",          "Kontynuuj",         "Pomiń",              "Wycofaj się",         "Tak",         "Nie",       "Czy jesteś pewien?"     }, // 86 Polish
        { "توقف",           "ادامه",             "رد کردن",            "عقب‌نشینی",           "بلی",         "نخیر",      "آیا مطمئن هستید؟"       }, // 87 Dari
        { "درېدنه",         "دوام ورکړئ",        "تېرول",              "شاتګ",                "هو",          "نه",        "ایا ډاډه یاست؟"         }, // 88 Pashto
        { "PAUSA",          "Continuar",         "Saltar",             "Retirar",             "Sim",         "Não",       "Tem a certeza?"         }, // 89 Portuguese
        { "PAUSA",          "Continuar",         "Pular",              "Recuar",              "Sim",         "Não",       "Tem certeza?"           }, // 90 Brazilian Portuguese
        { "PAUSE",          "Katux",             "K'ay",               "Withdraw",            "Yes",         "No",        "Are you sure?"          }, // 91 K'iche
        { "PAUSA",          "Katiy",             "Saqiy",              "Kutiy",               "Arí",         "Mana",      "Ancha allinchu?"        }, // 92 Quechua
        { "PAUSA",          "Continua",          "Sari peste",         "Retrair",             "Gea",         "Na",        "Es ti segir?"           }, // 93 Romansh
        { "PAUZĂ",          "Continuă",          "Sari peste",         "Retragere",           "Da",          "Nu",        "Ești sigur?"            }, // 94 Romanian
        { "ПАУЗА",          "Продолжить",        "Пропустить",         "Отступить",           "Да",          "Нет",       "Вы уверены?"            }, // 95 Russian
        { "PAUSE",          "Komeza",            "Simbuka",            "Gusubira inyuma",     "Yego",        "Oya",       "Uzi neza?"              }, // 96 Kinyarwanda
        { "विराम",          "जारी रखें",           "छोड़ें",               "निवृत्तिः",            "आम्",         "न",         "निश्चितम्?"             }, // 97 Sanskrit
        { "ТОКТООХ",        "Үргэлжлүүлэх",      "Алгасах",            "Тэрит",               "Эбэтэр",      "Суох",      "Итэҕэйэҕин дуо?"        }, // 98 Yakut
        { "روڪ",            "جاري رکو",          "ڇڏي ڏيو",            "واپسي",               "ها",          "نه",        "ڇا توهان کي پڪ آهي؟"    }, // 99 Sindhi
        { "PAUSE",          "Joatkke",           "Njuikut",            "Geassádit",           "Juo",         "Ii",        "Leatgo sihkar?"         }, // 100 Sami Northern
        { "විරාමය",          "ඉදිරියට",             "මඟහරින්න",            "ආපසු යන්න",          "ඔව්",         "නැහැ",      "ඔබට විශ්වාසද?"          }, // 101 Sinhala
        { "PAUZA",          "Pokračovať",        "Preskočiť",          "Ustúpiť",             "Áno",         "Nie",       "Ste si istí?"           }, // 102 Slovak
        { "PREMOR",         "Nadaljuj",          "Preskoči",           "Umik",                "Da",          "Ne",        "Ste prepričani?"        }, // 103 Slovenian
        { "PAUSE",          "Joekse",            "Vaajtelh",           "Withdraw",            "Yes",         "No",        "Are you sure?"          }, // 104 Sami Southern
        { "PAUSE",          "Joatke",            "Njuolgadit",         "Withdraw",            "Yes",         "No",        "Are you sure?"          }, // 105 Sami Lule
        { "PAUSE",          "Joatke",            "Njuolgadit",         "Withdraw",            "Yes",         "No",        "Are you sure?"          }, // 106 Sami Inari
        { "PAUSE",          "Jatkke",            "Njuolgad",           "Withdraw",            "Yes",         "No",        "Are you sure?"          }, // 107 Sami Skolt
        { "PAUZA",          "Vazhdo",            "Kalo",               "Tërhiqu",             "Po",          "Jo",        "Je i sigurt?"           }, // 108 Albanian
        { "ПАУЗА",          "Настави",           "Прескочи",           "Повлачење",           "Да",          "Не",        "Да ли сте сигурни?"     }, // 109 Serbian
        { "PAUSE",          "Tsoela pele",       "Tlola",              "Kgutlela",            "E",           "Tjhe",      "Na o na le bonnete?"    }, // 110 Sesotho
        { "PAUS",           "Fortsätt",          "Hoppa över",         "Dra dig tillbaka",    "Ja",          "Nej",       "Är du säker?"           }, // 111 Swedish
        { "PAUSE",          "Endelea",           "Ruka",               "Ondoka",              "Ndiyo",       "Hapana",    "Una uhakika?"           }, // 112 Kiswahili
        { "ܫܠܝܐ",            "ܐܫܬܘܕܝ",             "ܕܠܓ",                "ܢܦܘܩܝܐ",              "ܐܝܢ",         "ܠܐ",        "ܒܛܝܠ ܠܟ؟"               }, // 113 Syriac
        { "இடைநிறுத்தம்",      "தொடரவும்",           "தவிர்க்கவும்",          "பின்வாங்கு",          "ஆம்",         "இல்லை",      "உறுதியாக இருக்கிறீர்களா?" }, // 114 Tamil
        { "విరామం",          "కొనసాగించు",          "దాటవేయి",             "వెనక్కి తగ్గు",         "అవును",       "కాదు",      "మీకు ఖచ్చితంగా తెలుసా?"   }, // 115 Telugu
        { "PAUSE",          "Идома додан",       "Гузаштан",           "Ақибнишинӣ",          "Ҳа",          "Не",        "Оё шумо мутмаин ҳастед?" }, // 116 Tajik
        { "หยุดชั่วคราว",        "ดำเนินการต่อ",          "ข้าม",                "ถอนตัว",              "ใช่",         "ไม่",       "คุณแน่ใจหรือไม่?"        }, // 117 Thai
        { "PAUZA",          "Dowam et",          "Geç",                "Yza çekilmek",        "Hawa",        "Ýok",       "Ynanýarsyňyzmy?"        }, // 118 Turkmen
        { "PAUSE",          "Magpatuloy",        "Laktawan",           "Umurong",             "Oo",          "Hindi",     "Sigurado ka ba?"        }, // 119 Tagalog
        { "PAUSE",          "Tswela pele",       "Tlola",              "Boela",               "Ee",          "Nnyaa",     "A o na le bonnete?"     }, // 120 Tswana
        { "DURAKLAT",       "Devam et",          "Atla",               "Geri Çekil",          "Evet",        "Hayır",     "Emin misiniz?"          }, // 121 Turkish
        { "ПАУЗА",          "Дәвам итү",         "Калдыру",            "Чигенү",              "Әйе",         "Юк",        "Ышанасызмы?"            }, // 122 Tatar
        { "PAUSE",          "Ar tsenna",         "Skip",               "Withdraw",            "Yes",         "No",        "Are you sure?"          }, // 123 Tamazight
        { "توختىتىش",       "داۋاملاشتۇرۇش",      "ئاتلاپ ئۆتۈش",        "چېكىنىش",             "ھەئە",        "ياق",       "ئىشەندىڭىزمۇ؟"          }, // 124 Uyghur
        { "ПАУЗА",          "Продовжити",        "Пропустити",         "Відступити",          "Так",         "Ні",        "Ви впевнені?"           }, // 125 Ukrainian
        { "توقف",           "جاری رکھیں",        "چھوڑ دیں",           "واپسی",               "ہاں",         "نہیں",      "کیا آپ کو یقین ہے؟"     }, // 126 Urdu
        { "PAUSE",          "Davom etish",       "O‘tkazib yuborish",  "Chekinish",           "Ha",          "Yo'q",      "Ishonchingiz komilmi?"  }, // 127 Uzbek
        { "TẠM DỪNG",       "Tiếp tục",          "Bỏ qua",             "Rút lui",             "Có",          "Không",     "Bạn có chắc không?"     }, // 128 Vietnamese
        { "PAUSE",          "Kontine",           "Skip",               "Withdraw",            "Yes",         "No",        "Are you sure?"          }, // 129 Wolof
        { "NQAMAMA",        "Qhubeka",           "Tsiba",              "Rhoxa",               "Ewe",         "Hayi",      "Uqinisekile?"           }, // 130 Xhosa
        { "פּויזע",          "פאָרזעצן",           "איבערשפּרינגען",       "אַרויסציִען",          "יאָ",         "ניין",      "ביסטו זיכער?"           }, // 131 Yiddish
        { "PAUSE",          "Tẹsiwaju",          "Foju kọja",          "Yọkuro",              "Bẹẹni",       "Rara",      "Ṣe o da ọ loju?"        }, // 132 Yoruba
        { "MISA",           "Qhubeka",           "Yeqa",               "Hoxa",                "Yebo",        "Cha",       "Uqinisekile?"           }, // 133 Zulu
    };
    if (language < 0 || language >= 134) language = 0; // English fallback
    return table[language];
}

}
