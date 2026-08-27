/*
    Copyright 2016-2023 melonDS team

    This file is part of melonDS.

    melonDS is free software: you can redistribute it and/or modify it under
    the terms of the GNU General Public License as published by the Free
    Software Foundation, either version 3 of the License, or (at your option)
    any later version.

    melonDS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with melonDS. If not, see http://www.gnu.org/licenses/.
*/

#include "PauseMenuOverlay.h"
#include "InnerShadowPainter.h"

#include <QPainter>
#include <QTimer>
#include <cmath>
#include <QPainterPath>
#include <QTransform>
#include <QPixmap>

struct CutsceneMenuStrings { const char* title; const char* cont; const char* skip; };
static CutsceneMenuStrings cutsceneMenuStrings(int language)
{
    static const CutsceneMenuStrings table[134] = {
        { "PAUSE",          "Continue",          "Skip"              }, // 0  English
        { "ポーズ",          "つづける",           "スキップ"            }, // 1  Japanese
        { "PAUSE",          "Continuer",         "Passer"            }, // 2  French
        { "PAUSE",          "Fortfahren",        "Überspringen"      }, // 3  German
        { "PAUSA",          "Continua",          "Salta"             }, // 4  Italian
        { "PAUSA",          "Continuar",         "Saltar"            }, // 5  Spanish
        { "暂停",            "继续",               "跳过"              }, // 6  Chinese
        { "POUSE",          "Gaan voort",        "Oorslaan"          }, // 7  Afrikaans
        { "ማቆም",           "ቀጥል",              "ዝለል"               }, // 8  Amharic
        { "إيقاف مؤقت",     "متابعة",            "تخطي"              }, // 9  Arabic
        { "PAUSE",          "Continue",          "Skip"              }, // 10 Mapudungun
        { "إيقاف مؤقت",     "متابعة",            "تخطي"              }, // 11 Moroccan Arabic
        { "বিৰতি",            "চলাই যাওক",            "এৰি যাওক"            }, // 12 Assamese
        { "PAUSE",          "Davam et",          "Keç"               }, // 13 Azerbaijani
        { "ПАУЗА",          "Давам",             "Атлап үтеү"        }, // 14 Bashkir
        { "ПАЎЗА",          "Працягнуць",        "Прапусціць"        }, // 15 Belarusian
        { "ПАУЗА",          "Продължи",          "Пропусни"          }, // 16 Bulgarian
        { "বিরতি",            "চালিয়ে যান",            "এড়িয়ে যান"            }, // 17 Bengali
        { "མཚམས",          "མུ་མཐུད",              "མཆོང"               }, // 18 Tibetan
        { "PAUSE",          "Kenderc'hel",       "Lammat"            }, // 19 Breton
        { "PAUZA",          "Nastavi",           "Preskoči"          }, // 20 Bosnian
        { "PAUSA",          "Continua",          "Salta"             }, // 21 Catalan
        { "وەستان",         "بەردەوامبە",        "بازبدە"            }, // 22 Central Kurdish
        { "PAUSA",          "Cuntinua",          "Salta"             }, // 23 Corsican
        { "PAUZA",          "Pokračovat",        "Přeskočit"         }, // 24 Czech
        { "SAIB",           "Parhau",            "Neidio"            }, // 25 Welsh
        { "PAUSE",          "Fortsæt",           "Spring over"       }, // 26 Danish
        { "PAUS",           "Fortsäize",         "Iwwersprangen"     }, // 27 Lower Sorbian
        { "ވަކިކުރުން",           "ކުރިއަށް",               "ދޫކުރުން"              }, // 28 Divehi
        { "ΠΑΥΣΗ",          "Συνέχεια",          "Παράλειψη"         }, // 29 Greek
        { "PAUS",           "Jätka",             "Jäta vahele"       }, // 30 Estonian
        { "PAUS",           "Jätka",             "Jäta vahele"       }, // 31 Basque
        { "توقف",           "ادامه",             "پرش"               }, // 32 Persian
        { "TAUKO",          "Jatka",             "Ohita"             }, // 33 Finnish
        { "PAUSE",          "Magpatuloy",        "Laktawan"          }, // 34 Filipino
        { "PAUSE",          "Halda fram",        "Sleppa"            }, // 35 Faroese
        { "PAUSE",          "Fortsätt",          "Hoppa över"        }, // 36 Frisian
        { "PAUSE",          "Lean ort",          "Scipeáil"          }, // 37 Irish
        { "PAUSE",          "Lean ort",          "Leum thairis"      }, // 38 Scottish Gaelic
        { "PAUSE",          "Continue",          "Skip"              }, // 39 Gilbertese
        { "PAUSA",          "Continuar",         "Saltar"            }, // 40 Galician
        { "PAUSE",          "Weiter",            "Überspringen"      }, // 41 Swiss German
        { "વિરામ",           "ચાલુ રાખો",           "છોડી દો"             }, // 42 Gujarati
        { "PAUSE",          "Ci gaba",           "Tsallake"          }, // 43 Hausa
        { "השהיה",          "המשך",              "דלג"               }, // 44 Hebrew
        { "विराम",          "जारी रखें",           "छोड़ें"               }, // 45 Hindi
        { "PAUZA",          "Nastavi",           "Preskoči"          }, // 46 Croatian
        { "PAUS",           "Pokračować",        "Přeskočić"         }, // 47 Upper Sorbian
        { "SZÜNET",         "Folytatás",         "Kihagyás"          }, // 48 Hungarian
        { "ԴԱԴԱՐ",          "Շարունակել",        "Բաց թողնել"        }, // 49 Armenian
        { "JEDA",           "Lanjutkan",         "Lewati"            }, // 50 Indonesian
        { "PAUSE",          "Gaa n'ihu",         "Mafee"             }, // 51 Igbo
        { "ꀧꅇ",             "ꆏꌠ",                "ꀋꆏ"                 }, // 52 Yi
        { "HLÉ",            "Halda áfram",       "Sleppa"            }, // 53 Icelandic
        { "ᐃᓱᒪᖅ",          "ᑲᔪᓯᓗᑎᑦ",             "ᐃᓕᕋᐃᑦ"            }, // 54 Inuktitut
        { "პაუზა",          "გაგრძელება",        "გამოტოვება"        }, // 55 Georgian
        { "ПАУЗА",          "Жалғастыру",        "Өткізу"            }, // 56 Kazakh
        { "PAUSE",          "Nangma",            "Skip"              }, // 57 Greenlandic
        { "ផ្អាក",            "បន្ត",                 "រំលង"               }, // 58 Khmer
        { "ವಿರಾಮ",           "ಮುಂದುವರಿಸಿ",           "ಬಿಟ್ಟುಹೋಗಿ"            }, // 59 Kannada
        { "일시정지",        "계속",               "건너뛰기"            }, // 60 Korean
        { "विराम",          "जारी ठेवा",          "वगळा"              }, // 61 Konkani
        { "وەستان",         "بەردەوامبە",        "بازبدە"            }, // 62 Kurdish
        { "ՏԱԴԱՐ",          "Շարունակել",        "Բաց թողնել"        }, // 63 Kyrgyz
        { "PAUS",           "Weider",            "Iwwersprangen"     }, // 64 Luxembourgish
        { "ຢຸດ",            "ສືບຕໍ່",            "ຂ້າມ"              }, // 65 Lao
        { "PAUZĖ",          "Tęsti",             "Praleisti"         }, // 66 Lithuanian
        { "PAUZE",          "Turpināt",          "Izlaist"           }, // 67 Latvian
        { "Oki",            "Haere tonu",        "Tīpoka"            }, // 68 Māori
        { "ПАУЗА",          "Продолжи",          "Прескокни"         }, // 69 Macedonian
        { "വിരാമം",           "തുടരുക",             "ഒഴിവാക്കുക"           }, // 70 Malayalam
        { "ЗОГСООХ",        "Үргэлжлүүлэх",      "Алгасах"           }, // 71 Mongolian
        { "PAUSE",          "Continue",          "Skip"              }, // 72 Mohawk
        { "विराम",          "सुरू ठेवा",            "वगळा"              }, // 73 Marathi
        { "PAUSE",          "Fortsæt",           "Spring over"       }, // 74 Malay
        { "PAUSE",          "Kompli",            "Aqbeż"             }, // 75 Maltese
        { "ခဏရပ်",          "ဆက်ရန်",            "ကျော်ရန်"            }, // 76 Burmese
        { "PAUSE",          "Fortsett",          "Hopp over"         }, // 77 Norwegian (Bokmål)
        { "विराम",          "जारी राख्नुहोस्",      "छोड्नुहोस्"           }, // 78 Nepali
        { "PAUZE",          "Doorgaan",          "Overslaan"         }, // 79 Dutch
        { "PAUSE",          "Hald fram",         "Hopp over"         }, // 80 Norwegian (Nynorsk)
        { "PAUSE",          "Fortsett",          "Hopp over"         }, // 81 Norwegian
        { "PAUSA",          "Continuar",         "Saltar"            }, // 82 Occitan
        { "ବିରତି",           "ଜାରି ରଖନ୍ତୁ",          "ଛାଡ଼ନ୍ତୁ"             }, // 83 Odia
        { "PAUSE",          "Kontinuá",          "Skipe"             }, // 84 Papiamento
        { "ਵਿਰਾਮ",           "ਜਾਰੀ ਰੱਖੋ",             "ਛੱਡੋ"                }, // 85 Punjabi
        { "PAUZA",          "Kontynuuj",         "Pomiń"             }, // 86 Polish
        { "توقف",           "ادامه",             "رد کردن"           }, // 87 Dari
        { "درېدنه",         "دوام ورکړئ",        "تېرول"             }, // 88 Pashto
        { "PAUSA",          "Continuar",         "Saltar"            }, // 89 Portuguese
        { "PAUSA",          "Continuar",         "Pular"             }, // 90 Brazilian Portuguese
        { "PAUSE",          "Katux",             "K'ay"              }, // 91 K'iche
        { "PAUSA",          "Katiy",             "Saqiy"             }, // 92 Quechua
        { "PAUSA",          "Continua",          "Sari peste"        }, // 93 Romansh
        { "PAUZĂ",          "Continuă",          "Sari peste"        }, // 94 Romanian
        { "ПАУЗА",          "Продолжить",        "Пропустить"        }, // 95 Russian
        { "PAUSE",          "Komeza",            "Simbuka"           }, // 96 Kinyarwanda
        { "विराम",          "जारी रखें",           "छोड़ें"               }, // 97 Sanskrit
        { "ТОКТООХ",        "Үргэлжлүүлэх",      "Алгасах"           }, // 98 Yakut
        { "روڪ",            "جاري رکو",          "ڇڏي ڏيو"           }, // 99 Sindhi
        { "PAUSE",          "Joatkke",           "Njuikut"           }, // 100 Sami Northern
        { "විරාමය",          "ඉදිරියට",             "මඟහරින්න"           }, // 101 Sinhala
        { "PAUZA",          "Pokračovať",        "Preskočiť"         }, // 102 Slovak
        { "PREMOR",         "Nadaljuj",          "Preskoči"          }, // 103 Slovenian
        { "PAUSE",          "Joekse",            "Vaajtelh"          }, // 104 Sami Southern
        { "PAUSE",          "Joatke",            "Njuolgadit"        }, // 105 Sami Lule
        { "PAUSE",          "Joatke",            "Njuolgadit"        }, // 106 Sami Inari
        { "PAUSE",          "Jatkke",            "Njuolgad"          }, // 107 Sami Skolt
        { "PAUZA",          "Vazhdo",            "Kalo"              }, // 108 Albanian
        { "ПАУЗА",          "Настави",           "Прескочи"          }, // 109 Serbian
        { "PAUSE",          "Tsoela pele",       "Tlola"             }, // 110 Sesotho
        { "PAUS",           "Fortsätt",          "Hoppa över"        }, // 111 Swedish
        { "PAUSE",          "Endelea",           "Ruka"              }, // 112 Kiswahili
        { "ܫܠܝܐ",            "ܐܫܬܘܕܝ",             "ܕܠܓ"               }, // 113 Syriac
        { "இடைநிறுத்தம்",      "தொடரவும்",           "தவிர்க்கவும்"          }, // 114 Tamil
        { "విరామం",          "కొనసాగించు",          "దాటవేయి"            }, // 115 Telugu
        { "PAUSE",          "Идома додан",       "Гузаштан"          }, // 116 Tajik
        { "หยุดชั่วคราว",        "ดำเนินการต่อ",          "ข้าม"                }, // 117 Thai
        { "PAUZA",          "Dowam et",          "Geç"                }, // 118 Turkmen
        { "PAUSE",          "Magpatuloy",        "Laktawan"           }, // 119 Tagalog
        { "PAUSE",          "Tswela pele",       "Tlola"              }, // 120 Tswana
        { "DURAKLAT",       "Devam et",          "Atla"               }, // 121 Turkish
        { "ПАУЗА",          "Дәвам итү",         "Калдыру"            }, // 122 Tatar
        { "PAUSE",          "Ar tsenna",         "Skip"               }, // 123 Tamazight
        { "توختىتىش",       "داۋاملاشتۇرۇش",      "ئاتلاپ ئۆتۈش"        }, // 124 Uyghur
        { "ПАУЗА",          "Продовжити",        "Пропустити"         }, // 125 Ukrainian
        { "توقف",           "جاری رکھیں",        "چھوڑ دیں"           }, // 126 Urdu
        { "PAUSE",          "Davom etish",       "O‘tkazib yuborish"  }, // 127 Uzbek
        { "TẠM DỪNG",       "Tiếp tục",          "Bỏ qua"             }, // 128 Vietnamese
        { "PAUSE",          "Kontine",           "Skip"               }, // 129 Wolof
        { "NQAMAMA",        "Qhubeka",           "Tsiba"              }, // 130 Xhosa
        { "פּויזע",          "פאָרזעצן",           "איבערשפּרינגען"      }, // 131 Yiddish
        { "PAUSE",          "Tẹsiwaju",          "Foju kọja"          }, // 132 Yoruba
        { "MISA",           "Qhubeka",           "Yeqa"               }, // 133 Zulu
    };
    if (language < 0 || language >= 134) language = 0; // English fallback
    return table[language];
}

// Paints the Skip/Continue menu in device (pixel) coordinates over a w*h area.
// 't' is the animation clock in seconds, used to drive the hand bob and the glow
// orbit so the selected entry matches the live KH2 pause menu. 'language' selects the
// localized labels (firmware Language order; see cutsceneMenuStrings).
static void paintPauseMenu(QPainter& p, int w, int h, int selection, double t, int language, double menuSizeModifier)
{
    const CutsceneMenuStrings strings = cutsceneMenuStrings(language);

    static const QPixmap handCursorPixmap(":/ds/menu_hand.png");
    static const QPixmap glowPixmap(":/ds/menu_light.png");
    static const QPixmap selectedButtonPixmap(":/ds/button_selected.png");
    static const QPixmap unselectedButtonPixmap(":/ds/button_unselected.png");
    static const QPixmap pauseLabelPixmap(":/ds/pause_label.png");

    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);

    // Darken the paused frame behind the menu
    p.fillRect(QRect(0, 0, w, h), QColor(0, 0, 0, 120));

    const int centerX = w / 2;

    double pauseSizeModifier = (15.0/13.0) * menuSizeModifier;
    double buttonsSizeModifier = 1.25 * menuSizeModifier;

    double bottomMargin = 0.03 * menuSizeModifier;
    double titleCenterY = h * (0.5 - bottomMargin - 0.13 * menuSizeModifier);
    double firstButtonY = h * (0.5 - bottomMargin);

    // "PAUSE" title banner. The asset provides the decorative rule lines above/below the
    // title; the title text itself is drawn on top, in the stylised KH title font, and is
    // localized per language (like the button labels below).
    if (!pauseLabelPixmap.isNull()) {
        const qreal titleImageHeight = h * 0.15 * pauseSizeModifier;
        const qreal titleImageWidth = titleImageHeight * pauseLabelPixmap.width() / pauseLabelPixmap.height();
        const QRectF titleImageRect(centerX - titleImageWidth / 2.0, titleCenterY - titleImageHeight / 2.0,
                                     titleImageWidth, titleImageHeight);
        p.drawPixmap(titleImageRect, pauseLabelPixmap, pauseLabelPixmap.rect());
    }

    QFont titleFont("KHGummi");
    qreal titlePixelSize = h * 0.064 * pauseSizeModifier;
    titleFont.setPixelSize((int)titlePixelSize);
    p.setFont(titleFont);

    const QString titleText = QString::fromUtf8(strings.title);
    QFontMetrics titleFontMetrics(titleFont);
    qreal titleTextWidth = titleFontMetrics.horizontalAdvance(titleText);

    // Build the glyphs as a path to make changes to it
    qreal titleBaselineY = ((int)titleCenterY) + (titleFontMetrics.ascent() - titleFontMetrics.descent()) / 2.0;
    QPainterPath titlePath;
    titlePath.addText(centerX - titleTextWidth / 2.0, titleBaselineY, titleFont, titleText);

    // Squeeze the text horizontally by 20%
    QTransform titleSqueeze;
    titleSqueeze.translate(centerX, 0.0).scale(0.8, 1.0).translate(-centerX, 0.0);
    titlePath = titleSqueeze.map(titlePath);

    // Black outline
    const qreal titleOutlineWidth = titlePixelSize * 0.15 * pauseSizeModifier;
    p.strokePath(titlePath, QPen(QColor(0, 0, 0), titleOutlineWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    // Text color
    p.fillPath(titlePath, QColor(255, 255, 255));

    static QImage cachedTitleShadow;
    static QPointF cachedTitleShadowPos;
    static QString cachedTitleShadowText;
    static QRectF cachedTitleShadowBounds;
    const QRectF titleBounds = titlePath.boundingRect();
    if (cachedTitleShadowText != titleText || cachedTitleShadowBounds != titleBounds) {
        const int blurRadius = (int)(titlePixelSize * 0.03 * pauseSizeModifier);
        cachedTitleShadow = InnerShadowPainter::renderInnerBorderShadow(titlePath, blurRadius, 235, &cachedTitleShadowPos);
        cachedTitleShadowText = titleText;
        cachedTitleShadowBounds = titleBounds;
    }
    p.drawImage(cachedTitleShadowPos, cachedTitleShadow);

    // Buttons
    const QString buttonLabels[2] = { QString::fromUtf8(strings.cont), QString::fromUtf8(strings.skip) };
    int buttonHeight = (int)(h * 0.075 * buttonsSizeModifier);
    const QPixmap& referenceButtonPixmap = !selectedButtonPixmap.isNull() ? selectedButtonPixmap : unselectedButtonPixmap;
    const qreal buttonAspectRatio = (!referenceButtonPixmap.isNull() && referenceButtonPixmap.height() > 0)
        ? (qreal)referenceButtonPixmap.width() / referenceButtonPixmap.height() : 5.3;
    int buttonWidth = (int)(buttonHeight * buttonAspectRatio);
    int buttonSpacing = (int)(buttonHeight * 0.175);

    QFont buttonFont("DFSouGei-W5G-KH25");
    buttonFont.setPixelSize((int)(h * 0.056 * buttonsSizeModifier));
    // Some localized labels are wider than the English ones (e.g. German "Überspringen").
    // Shrink the font until the widest label fits the button's flat area - the rounded caps
    // eat ~buttonHeight of width - so labels stay inside the button rather than being clipped.
    {
        const qreal availableTextWidth = buttonWidth - buttonHeight;
        QFontMetrics shrinkFontMetrics(buttonFont);
        int widestLabelWidth = 0;
        for (const QString& label : buttonLabels) {
            widestLabelWidth = qMax(widestLabelWidth, shrinkFontMetrics.horizontalAdvance(label));
        }
        if (widestLabelWidth > availableTextWidth && availableTextWidth > 0) {
            buttonFont.setPixelSize((int)(buttonFont.pixelSize() * availableTextWidth / widestLabelWidth));
        }
    }
    p.setFont(buttonFont);
    const QFontMetrics buttonFontMetrics(buttonFont);

    for (int i = 0; i < 2; i++)
    {
        QRect buttonRect(centerX - buttonWidth / 2, ((int)firstButtonY) + i * (buttonHeight + buttonSpacing),
                          buttonWidth, buttonHeight);
        bool isSelected = (selection == i);

        const QPixmap& buttonBackgroundPixmap = isSelected ? selectedButtonPixmap : unselectedButtonPixmap;
        if (!buttonBackgroundPixmap.isNull()) {
            p.drawPixmap(buttonRect, buttonBackgroundPixmap, buttonBackgroundPixmap.rect());
        }

        // Center on the label's own ink box rather than QFontMetrics::height/ascent - the
        // button font's vertical metrics don't line up with its glyphs' visual center, so
        // Qt::AlignCenter alone would sit noticeably off from the pill's middle.
        p.setBrush(Qt::NoBrush);
        const QString& labelText = buttonLabels[i];
        const QRect labelInkBox = buttonFontMetrics.tightBoundingRect(labelText); // relative to baseline (top is negative)
        const qreal labelTextWidth = buttonFontMetrics.horizontalAdvance(labelText);
        const qreal labelBaselineY = (buttonRect.center().y() - labelInkBox.top() - labelInkBox.height() / 2.0) - (h * 0.0025 * buttonsSizeModifier);
        const qreal labelX = buttonRect.center().x() - labelTextWidth / 2.0;

        // Drop shadow cast down-and-right, offset from the label itself.
        const qreal labelShadowOffset = buttonFontMetrics.height() * 0.07 * buttonsSizeModifier;
        p.setPen(QColor(0, 0, 0));
        p.drawText(QPointF(labelX + labelShadowOffset, labelBaselineY + labelShadowOffset), labelText);

        p.setPen(Qt::white);
        p.drawText(QPointF(labelX, labelBaselineY), labelText);

        if (isSelected) {
            // Glow accent. The KH2 glow doesn't circle the cap; it wanders a small
            // Lissajous figure (~4:5 frequency ratio) inside a square region in the
            // upper-right of the rounded cap. The constants below were measured from
            // the reference footage (relative to the cap centre / radius).
            if (!glowPixmap.isNull()) {
                const qreal capRadius = buttonHeight / 2.0;
                const qreal capCenterX = buttonRect.right() - capRadius;
                const qreal capCenterY = buttonRect.center().y();
                const qreal glowBoxCenterX = capCenterX + 0.19 * capRadius;
                const qreal glowBoxCenterY = capCenterY - 0.56 * capRadius;
                const qreal glowAmplitudeX = 0.55 * capRadius;
                const qreal glowAmplitudeY = 0.50 * capRadius;
                // Scale both frequencies together so the 4:5 figure keeps its shape,
                // just traced more slowly than the reference.
                const qreal glowSpeed = 0.5;
                const qreal glowX = glowBoxCenterX + glowAmplitudeX * std::cos(2.0 * M_PI * (1.012 * glowSpeed) * t - 2.39);
                const qreal glowY = glowBoxCenterY + glowAmplitudeY * std::cos(2.0 * M_PI * (1.265 * glowSpeed) * t - 1.57);

                int glowSize = (int)(buttonHeight * 0.5);
                p.drawPixmap(QRectF(glowX - glowSize / 2.0, glowY - glowSize / 2.0, glowSize, glowSize), glowPixmap,
                             glowPixmap.rect());
            }
            // Pointing-hand cursor to the left of the selected button, beckoning with a
            // small horizontal bob toward the entry.
            if (!handCursorPixmap.isNull()) {
                qreal handAspectRatio = handCursorPixmap.height() > 0
                    ? (qreal)handCursorPixmap.width() / handCursorPixmap.height() : 1.0;
                int handHeight = (int)(buttonHeight * 1.15);
                int handWidth = (int)(handHeight * handAspectRatio);
                qreal handBobOffset = buttonHeight * 0.15 * std::sin(t * (2.0 * M_PI / 1.2));
                int handX = buttonRect.left() - handWidth + (int)(buttonHeight * 0.20 + handBobOffset);
                int handY = buttonRect.center().y() - handHeight / 2;
                p.drawPixmap(QRect(handX, handY, handWidth, handHeight), handCursorPixmap);
            }
        }
    }
}

PauseMenuOverlay::PauseMenuOverlay(QWidget* parent) :
    QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setFocusPolicy(Qt::NoFocus);
    hide();

    // ~60 FPS repaint while the menu is up, so the hand/glow animations stay smooth.
    m_animTimer = new QTimer(this);
    m_animTimer->setInterval(16);
    connect(m_animTimer, &QTimer::timeout, this, [this]() { update(); });
}

void PauseMenuOverlay::setMenuVisible(bool visible)
{
    if (m_visible == visible) {
        return;
    }
    m_visible = visible;
    if (visible) {
        m_animClock.restart();
        m_animTimer->start();
    } else {
        m_animTimer->stop();
    }
}

void PauseMenuOverlay::setSelection(int selection)
{
    m_selection = selection;
    if (m_visible) {
        update();
    }
}

void PauseMenuOverlay::setLanguage(int language)
{
    m_language = language;
    if (m_visible) {
        update();
    }
}

void PauseMenuOverlay::setSizeModifier(double modifier)
{
    m_sizeModifier = modifier;
    if (m_visible) {
        update();
    }
}

void PauseMenuOverlay::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    paint(painter, width(), height());
}

QImage PauseMenuOverlay::renderToImage(const QSize& pixelSize) const
{
    QImage image(pixelSize, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    paint(painter, pixelSize.width(), pixelSize.height());
    return image;
}

void PauseMenuOverlay::paint(QPainter& painter, int w, int h) const
{
    const double t = m_animClock.isValid() ? m_animClock.elapsed() / 1000.0 : 0.0;
    paintPauseMenu(painter, w, h, m_selection, t, m_language, m_sizeModifier);
}
