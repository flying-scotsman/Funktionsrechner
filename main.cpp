/*
    Ein Funktionsrechner -- eine einfache Implementierung eines arithmetischen Parsers

    Der Shunting-Yard-Algorithmus wird hier angewendet, ohne Operatoren-Pr�ferenz,
    da die Ausdr�cke vollst�ndig geklammert sind. Anstatt dem Stapel-Konzept wird hier
    das std::vector benutzt, da im Zusatz zu push/pop F�higkeiten kann ein std::vector
    auch geleert werden.

    Syntaxfehler und andere Fehler (nicht numerische Charaktere) werden detektiert und
    angezeigt. Unn�tige aber nicht falsche Mehrklammerung ist erlaubt.

    Matthew F. B. Green

    11.03.17
*/


#include <iostream>
#include <fstream>
#include <vector> // vector
#include <string> // string
#include <cctype> // isdigit

using namespace std;

// TODO:
// Too many (correct) brackets should not cause a crash
// What about negative numbers?

void evaluate(ifstream& str_);
void performOperation(vector<float>& vals_, vector<char>& ops_);
void endOfLineProcedure(ifstream& str_, vector<float>& vals_, vector<char>& ops_, string& num_, int& bcount_);
void pushAndClear(vector<float>& vals_, string& num_);
void lineClear(ifstream& str_, vector<float>& vals_, vector<char>& ops_, string& num_, int& bcount_);

int main(int argc, char** argv) {
    if(argc > 1) {
        // Stream wird ge�ffnet
        ifstream str(argv[1]);

        // Es wird getestet ob Stream nicht ge�ffnet wurde
        if(!str.is_open()) cout << "Fehler: Datei konnte nicht gefunden oder geoeffnet werden." << endl;
        // Jetzt k�nnen die Gleichungen in der eingegeben Datei evaluiert werden
        else evaluate(str);

        // Stream wird geschlossen
        str.close();
    }
    else cout << "Fehler: Kein Dateiname wurde eingegeben." << endl;

    return 0;
}

// Shunting-Yard Algorithmus zur Arithmetik-Evaluierung
// Werte und Operationen werden nach und nach in Vektoren gemacht und
// in der richtigen Reihenfolge, von Klammern diktiert, evaluiert
void evaluate(ifstream& str_) {
    // Zwei Vektoren werden deklariert f�r die Werte und die Operatoren
    vector<float> vals;
    vector<char> ops;

    // num stellt die einzulesende Zahl dar
    // num ist ein String weil ein Punkt und/oder beliebig viel Nachkommastellen k�nnen kommen
    // Bevor num in den Wert-Vektor kommt, wird es in ein Float umgewandelt
    string num;

    // Die Anzahl von Klammern wird verfolgt
    int bcount;

    // Ein Charakter nach dem anderen wird eingelesen
    char c;
    while(str_.get(c)) {
        // Wenn ein Kommentar kommt, wird die Linie �bersprungen
        if(c == '#') lineClear(str_, vals, ops, num, bcount);
        // c ist entweder eine �ffnende Klammer oder ein Operator
        // Da Terme vollst�ndig geklammert sind, ist die Reihenfolge von Operatoren irrelevant
        else if(c == '(' || c == '+' || c == '-' || c == '*' || c == '/') {
            // Wenn c ein Operator ist, f�ge es zum Operator-Vektor hinzu
            if(c != '(') ops.insert(ops.begin(), c);

            // Eine �ffnende Klammer - bcount wird erh�ht
            else bcount++;

            // Da die Zahl zu Ende ist, kann num in den Wert-Vektor getan werden
            if(!num.empty()) pushAndClear(vals, num);
        }
        // Die erste Operation in dem Operation-Vektor muss mit den
        // ersten Werten durchgef�hrt werden
        else if(c == ')') {
            // Eine schlie�ende Klammer - 1 wird von bcount abgezogen
            bcount--;

            // num kommt erst zum Wert-Vektor wenn es nicht leer ist
            if(!num.empty()) {
                pushAndClear(vals, num);
                // Dies k�mmert sich um die erste schlie�ende Klammer, falls es eine Reihe davon gibt
                if(!ops.empty()) performOperation(vals, ops);
            }
            // Operation wird ausgef�hrt wenn beide Vektoren nicht leer sind
            // ALLERDINGS wenn num leer ist und es noch Operationen im Vektor gibt,
            // obwohl die Klammerzahl nicht gleich 0 ist, sollte die Operation nicht ausgef�hrt werden
            // Dies erlaubt (unn�tige) Doppelklammern, wie in Term 5
            else if(!vals.empty() && !ops.empty() && bcount == 0) performOperation(vals, ops);
        }
        // Zeilenende - Operation wird eventuell ausgef�hrt
        else if(c == '\n') endOfLineProcedure(str_, vals, ops, num, bcount);
        else { // c sollte Teil einer Zahl sein
            // Falls c ein Punkt ist
            if(c == 46) num += ".";
            // Sonst wird der Ziffer zu num hinzugef�gt
            else {
                // Falls etwas nicht Numerisches in der Datei ist
                if(!isdigit(c)) {
                    cout << "Fehler: keine nicht numerischen Charaktere erlaubt." << endl;
                    lineClear(str_, vals, ops, num, bcount);
                }
                else num += to_string(c-48); // -48 wegen UTF-8 Kodierung
            }
        }
    }
    // Das Ende der Datei wurde erreicht - Operationen werden eventuell noch ausgef�hrt
    endOfLineProcedure(str_, vals, ops, num, bcount);
}

void endOfLineProcedure(ifstream& str_, vector<float>& vals_, vector<char>& ops_, string& num_, int& bcount_) {
    // Wenn die Klammeranzahl nicht aufgeht, ist das schlecht
    if(bcount_ != 0) {
        cout << "Syntaxfehler: fehlende Klammern." << endl;
        lineClear(str_, vals_, ops_, num_, bcount_);
    }
    // Wenn am Ende der Zeile eine Zahl noch nicht in den Wert-Vektor gekommen ist, wird das jetzt gemacht
    if(!num_.empty()) pushAndClear(vals_, num_);

    // Da num nur in den Wert-Vektor getan wird wenn ein Operator oder eine schlie�ende Klammer kommt,
    // kann es sein, dass noch eine Operation ausgef�hrt werden muss
    if(!ops_.empty()) performOperation(vals_, ops_);

    // Ergebnis wird anzeigt und der oberste Element des Wert-Vektors entfernt
    if(!vals_.empty()) {
        cout << vals_.front() << endl;
        vals_.erase(vals_.begin());
    }
}

void performOperation(vector<float>& vals_, vector<char>& ops_) {
    // Die ersten Elemente der Vektoren werden ausgew�hlt
    char op = ops_.front();
    float val = vals_.front();

    // Die ersten Elemente k�nnen jetzt entfernt werden
    ops_.erase(ops_.begin());
    vals_.erase(vals_.begin());

    // Operation wird ausgef�hrt - n�chster Wert in dem Vektor kommt logisch davor
    if(op == '+')   val = vals_.front() + val;
    if(op == '-')   val = vals_.front() - val;
    if(op == '*')   val = vals_.front() * val;
    if(op == '/')   val = vals_.front() / val;

    // Der zweite Wert kann jetzt entfernt werden
    vals_.erase(vals_.begin());

    // Der berechnete Wert val geht jetzt zum Vektor
    vals_.insert(vals_.begin(), val);
}

void pushAndClear(vector<float>& vals_, string& num_) {
    // num wird zum Anfang des Wert-Vektors hinzugef�gt
    vals_.insert(vals_.begin(), stof(num_));

    // num wird geleert
    num_.clear();
}

void lineClear(ifstream& str_, vector<float>& vals_, vector<char>& ops_, string& num_, int& bcount_) {
    // Der Rest der Zeile des Streams wird eingelesen
    string l;
    getline(str_, l);

    // Die Vektoren und num werden geleert, damit sie nicht
    // falscherweise weiterhin evaluiert werden in dieser Zeile
    num_.clear();
    vals_.clear();
    ops_.clear();

    // Count wird auf 0 gesetzt f�r die n�chste Zeile
    bcount_ = 0;
}
