// http://www.tutorialspoint.com/cplusplus/cpp_files_streams.htm
//NASDAQ Symbols download
//http://www.nasdaq.com/screening/companies-by-name.aspx?letter=0&exchange=nasdaq&render=download
//NYSE Symbols download
//http://www.nasdaq.com/screening/companies-by-name.aspx?letter=0&exchange=nyse&render=download
//ALL Symbols download
//http://www.nasdaq.com/screening/companies-by-name.aspx?letter=0&exchange=all&render=download

//C++ Files and Streams
#include <fstream>

//To dawnload symbols NASDAQ/NYSE
//http://stackoverflow.com/questions/5246843/how-to-get-a-complete-list-of-ticker-symbols-from-yahoo-finance
//http://stackoverflow.com/questions/5167625/splitting-a-c-stdstring-using-tokens-e-g?lq=1
//http://stackoverflow.com/questions/4533063/how-does-ifstreams-eof-work
#include <vector>
#include <string>
#include <sstream>
#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>

#include <pqxx/pqxx>

using namespace std;
using namespace pqxx;

int tlgetch() {
    struct termios oldtc, newtc;
    int ch;
    tcgetattr(STDIN_FILENO, &oldtc);
    newtc = oldtc;
    newtc.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newtc);
    ch=getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldtc);
    return ch;
}

const std::string EnterPassword(int lon) {
    char pw[20];
    char ch;
    int i = 0;

    cout << "Enter password: ";
//  my_getpass(pwd, 10, cin);
    ch = '\00';
    while(ch != '\n') //Loop until 'Enter' is pressed
    {
        ch = tlgetch();
        if (ch == 127)  {
            if (i > 0) {
                cout << '\b';  //Cursor moves 1 position backwards
                i -=1;
            }
            continue;
        }
        if (i<lon) {
            pw[i] = ch;
            if (ch != '\n') {
                cout << "*";
            }
            i += 1;
        }
    }
    pw[i - 1] = '\00';
    cout << endl;
    return pw;
}

//Ver: http://stackoverflow.com/questions/1120140/how-can-i-read-and-parse-csv-files-in-c
//CSV Format:
//Field separators: ','
//String delimiter: '"'

void ParseCSV(const string& csvSource, vector<string> &line ) //vector<vector<string> >& lines)
{
    bool inQuote(false);
    bool newLine(false);
    string field;
    line.clear();


    string::const_iterator aChar = csvSource.begin();
    while (aChar != csvSource.end())
    {
        switch (*aChar)
        {
        case '"':
            newLine = false;
            inQuote = !inQuote;
            break;

        case ',':
            newLine = false;
            if (inQuote == true)
            {
                field += *aChar;
            }
            else
            {
                line.push_back(field);
                field.clear();
            }
            break;

        case '\n':
        case '\r':
            if (inQuote == true)
            {
                field += *aChar;
            }
            else
            {
                if (newLine == false)
                {
                    line.push_back(field);
                    field.clear();
                    line.clear();
                    newLine = true;
                }
            }
            break;

        default:
            newLine = false;
            field.push_back(*aChar);
            break;
        }

        aChar++;
    }

    if (field.size())
        line.push_back(field);
}

// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
// Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
// for more information about date/time format

const std::string CurrentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);

    return buf;
}

const std::string FormatDateTime(std::string &Date, std::string &Time) {
    string     s, h, m;
    char       buf[80];
    struct tm  tm;
    int        l;

//   http://stackoverflow.com/questions/14099542/c-convert-date-formats
    s = Date.c_str();
    s.append(" ");
    l = Time.length();
    h = Time.substr(0, l - 2);
//    h = "0:00";
    if (h.substr(0,2) != "0:") {
        s.append(h.c_str());
        s.append(":00");
        m = Time.substr(l - 2, 2);
        s.append(m.c_str());
        strptime(s.c_str(), "%m/%d/%Y %I:%M:%S %p", &tm);
        strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tm);
    } else {
        s.append( "1" );
        s.append(h.substr(1,3));
        s.append(":00");
        m = Time.substr(l - 2, 2);
        s.append(m.c_str());
        strptime(s.c_str(), "%m/%d/%Y %I:%M:%S %p", &tm);
        tm.tm_hour = 0;
        strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tm);
    }
    return buf;
}

void Insert_Quotes(connection &C, vector<string> &line) {
    string sql;

    /* Create a psql transactional object. */
    work W(C);

    //0 s  = Symbol                   "CHAR10
    //1 n  = Name                     "CHAR80
    //2 d1 = Last Trade Date          "mm/dd/yyyy"
    //3 t1 = Last Trade Time          "hh:mmAM/PM"
    //4 a  = Ask                      FLOAT4
    //5 b  = Bid                      FLOAT4
    //6 l  = Last Trade (With Time)   CHAR(40) "hh:mmAM/PM"
    //7 l1 = Last Trade (Price Only)  FLOAT4
    //8 m  = Day's Average            CHAR40
    //9 v  = Volume                   INT4

    sql = "INSERT INTO QUOTES (SYMBOL, TSTAMP, TLAST, LAST_TRADE, ASK, BID, VOLUME) VALUES ( ";

    sql.append("'");
    sql.append(line.at(0)); //symbol s
    sql.append("', ");
    sql.append("to_timestamp('");
    sql.append(CurrentDateTime());
    sql.append("', 'yyyy-mm-dd hh24:mi:ss'), ");

    sql.append("to_timestamp('");
    sql.append(FormatDateTime(line.at(2), line.at(3)));
    sql.append("', 'yyyy-mm-dd hh24:mi:ss'), ");

    if (line.at(7) != "N/A") {
        sql.append(line.at(7)); //Last Trade (l1)
    }
    else {
        sql.append("0");
    }
    sql.append(", ");

    if (line.at(4) != "N/A") {
        sql.append(line.at(4)); //Ask Real Time (b2)
    }
    else {
        sql.append("0");
    }
    sql.append(", ");

    if (line.at(5) != "N/A") {
        sql.append(line.at(5)); //Bid Real Time (b3)

    }
    else {
        sql.append("0");
    }
    sql.append(", ");

    if (line.at(9) != "N/A") {
        sql.append(line.at(9)); //Volume (v)
    }
    else {
        sql.append("0");
    }
    sql.append(" );");

    /*Execute SQL query*/
    W.exec( sql );
    W.commit();
//    cout << sql << endl;
}

void Process_Quotes (connection &C, unsigned long &i) {
    std::string s;
    vector<string> line;
    string textFile;
    ifstream iFile;

    textFile = "./quotes.csv";
    iFile.open ( textFile.c_str());
    if (iFile.good()) {
        while (!iFile.eof()) {
            getline(iFile, s);
            ParseCSV(s, line);
            if (!line.empty()) {
                cout << "Num.:" << i << " " << s << endl;

//                 for (vector<string>::const_iterator iter = line.begin(); iter != line.end(); ++iter) {
//                     cout << *iter << endl;
//                 }
                Insert_Quotes(C, line);
            }
        }
    }
    iFile.close();

}

int main(int argc, char **argv) {
    string db, host, port, usr, pwd, con;
    int len;
    unsigned long i;

    if (argc ==  2) {
        if (string(argv[1]) == "-h") {
            //123456789|123456789|123456789|123456789|123456789|123456789|123456789|123456789|
            cout << "Trading Lab Receiver for trading quotes" << endl;
            cout << "Data are registered in PostgreSQL data base." << endl;
            cout << "Before tlrec execution call:" << endl;
            cout << "  $ postgres -D /usr/local/pgsql/data." << endl;
            cout << "Usage:" << endl;
            cout << "  tlrec -d<dbname> -h<hostaddr> -p<port> -u<user> -p[<password>]" << endl;
            cout << "General options:" << endl;
            cout << "  -d <dbname>   PostgreSQL data base name" << endl;
            cout << "  -h <hostaddr> Host address, for local host is 127.0.0.1" << endl;
            cout << "  -p aport>     Port, usually is 5432" << endl;
            cout << "  -u <user>     User,  database name registered" << endl;
            cout << "  -p<password>  Password, optional, when void manual entry" << endl;
            cout << "Example:" << endl;
            cout << "  $ ./tlrec -dmydb -h127.0.0.1 -p5432 -ujordi -p" << endl;

        }
        if (string(argv[1]) == "-v") {
            cout<< "tlrec (1.0.0)" << endl;
        }
        return 0;
    }
    if (argc !=  6) {
        cout << "Usage is tlrec -d<dbname> -h<hostaddr> -p<port> -u<user> -p[<password>]" << endl;
        cout << "General options:" << endl;
        cout << "  -h for help" << endl;
        cout << "  -v for version" << endl;
        return 0;
    }

    if (string(argv[1]).substr(0,2) == "-d") {
        len = string(argv[1]).size();
        db = string(argv[1]).substr(2,len-2);
    }

    if (string(argv[2]).substr(0,2) == "-h") {
        len = string(argv[2]).size();
        host = string(argv[2]).substr(2,len-2);
    }

    if (string(argv[3]).substr(0,2) == "-p") {
        len = string(argv[3]).size();
        port = string(argv[3]).substr(2,len-2);
    }

    if (string(argv[4]).substr(0,2) == "-u") {
        len = string(argv[4]).size();
        usr = string(argv[4]).substr(2,len-2);
    }

    if (string(argv[5]).substr(0,2) == "-p") {
        len = string(argv[5]).size();
        pwd = string(argv[5]).substr(2,len-2);
    }

//     for (int i = 0; i < argc; ++i) {
//         std::cout << argv[i] << std::endl;
//     }

    if (pwd == "") pwd = EnterPassword(10);

    con = "dbname=";
    con.append(db);
    con.append(" user=");
    con.append(usr);
    con.append(" password=");
    con.append(pwd);
    con.append(" hostaddr=");
    con.append(host);
    con.append(" port=");
    con.append(port);

    try {
//        connection C("dbname=mydb user=jordi password=XXXXXXXX hostaddr=127.0.0.1 port=5432");
        connection C(con.c_str());
        if (C.is_open()) {
            cout << "Opened database successfully: " << C.dbname() << endl;
        } else {
            cout << "Can't open database" << endl;
            return 1;
        }
        // s  = Symbol                   "CHAR10
        // n  = Name                     "CHAR80
        // d1 = Last Trade Date          "mm/dd/yyyy"
        // t1 = Last Trade Time          "hh:mmAM/PM"
        // a  = Ask                      FLOAT4
        // b  = Bid                      FLOAT4
        // l  = Last Trade (With Time)   CHAR(40)
        // l1 = Last Trade (Price Only)  FLOAT4
        // m  = Day's Average            CHAR40
        // v  = Volume                   INT4
        string command =  "wget -q -O quotes.csv \"http://finance.yahoo.com/d/quotes.csv?s=EURUSD=X+EURGBP=X+EURAUD=X+EURJPY=X+EURCHF=X+EURSEK=X+EURCAD=X+EURNZD=X+EURSGD=X+EURNOK=X&f=snd1t1abll1mv\"";
        //http://www.howtogeek.com/89360/how-to-view-stock-quotes-from-the-command-line/
        //curl -s 'http://download.finance.yahoo.com/d/quotes.csv?s=aapl&f=l1'
//        for(int i=0; i<20000; i++) {  //720 = 2h de actividad
	i = 1;
	while (true) {
            system(command.c_str());
            //http://www.cplusplus.com/forum/beginner/91389/
            //http://www.cplusplus.com/forum/beginner/104130/
            Process_Quotes(C, i);
            cout << endl;
            pqxx::internal::sleep_seconds(4);
	    i += 1;
        }
        C.disconnect ();
        cout << "Closed database successfully: " << C.dbname() << endl;
    } catch (const std::exception &e) {
        cerr << e.what() << std::endl;
        return 1;
    }
}
