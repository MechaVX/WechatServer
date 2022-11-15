#if 1
#include <iostream>
#include <sstream>
#include <mysql/mysql.h>
using namespace std;

#define MYSQL_DEFAULT_PORT 3306
const char *host = "localhost";
const char *user = "root";
const char *passwd = "123456";
string databasename = "Wechat";
int main(int argc, char const *argv[])
{
    MYSQL mysql;
    
    string sqlcmd;
    mysql_library_init(NULL, NULL, NULL);
    mysql_init(&mysql);
    MYSQL *ret = mysql_real_connect(&mysql, host, user, passwd, databasename.data(), MYSQL_DEFAULT_PORT, 0, 0);
    if (ret == NULL)
    {
        cout << "connect to mysql fail" << endl;
    }
    else
    {
        cout << "connect to mysql successfully, now input sql cmd" << endl;
        while (1)
        {
            getline(cin, sqlcmd);
            if (sqlcmd == "")
                continue;
            if (sqlcmd == "stop")
                break;
            int columns;
            cout << "input there will be how many columns of result:";
            cin >> columns;
            if (mysql_query(&mysql, sqlcmd.data()) != 0)
            {
                cout << mysql_error(&mysql) << endl;
                break;
            }
            //MYSQL_RES *result = mysql_use_result(&mysql);
            MYSQL_RES *result = mysql_store_result(&mysql);
            cout << (result == nullptr) << endl;
            MYSQL_ROW row;
            while (result && (row = mysql_fetch_row(result)) != NULL)
            {
                auto length = mysql_fetch_lengths(result);
                if (length == nullptr)
                    cout << "no result return" << endl;
                for (int i = 0; i < columns; ++i)
                {
                    cout << length[i] << ' ' << row[i] << ' ';
                }
                    
                cout << endl;
            }
            if (result != nullptr)
                mysql_free_result(result);
        }
    }
    
    mysql_close(&mysql);
    mysql_library_end();
    return 0;
}
#endif