# tlrec
tlrec reads the Yahoo Finance quotes for a predefined list of symbols and values.
reads quotes values every 10 second, and saves them into PostgreSQL table "quotes".
This program can run in background and saves data while running.

Program syntax, see tlrec help:

  $ tlrec -h


# Environment

* Operating System: openSUSE 13.1
* Development tool: KDevelop 4.5.2 for KDE 4.11.5
* Languaje: C++
* Program directory; HOME/TradingLab/tlrec
* PostgreSQL lib: libpqxx

# PostgreSQL config

Stept to setup PstgreSQL database:

* Create database mydb: 

* Start db service: 