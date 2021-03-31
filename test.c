#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <postgresql/libpq-fe.h>
#include <postgresql/libpq/libpq-fs.h>

int charger(char *file, char **buff);

int main()
{
    char *testFile;
    int testFilelen;

    testFilelen = charger("test.c", &testFile);

    printf("%s",testFile);
    printf("\nsize : %d\n", testFilelen);

    free(testFile);

    //----------------------------------------------------------
    PGconn *conn = PQconnectdb("host=localhost port=5432 dbname=idriss user=idriss password=idriss");
    PGresult *res;
    int i,j,nFields,fd;
    char oid[10];
    Oid OID;
    char * buff;

    if (PQstatus(conn) != CONNECTION_OK)
    {
        fprintf(stderr, "Connection to database failed: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
    }
    else
    {
        printf("connexion OK\n");
        /*
        res = PQexec(conn, "INSERT INTO test(name,file)\
                            VALUES('test',lo_import('/home/idriss/lab/postgres/test.c'))\
        ");
        if (PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            fprintf(stderr, "INSERT failed: %s", PQerrorMessage(conn));
        }
        PQclear(res);
        */
        res = PQexec(conn, "SELECT * FROM test");
        if (PQresultStatus(res) != PGRES_TUPLES_OK)
        {
            fprintf(stderr, "SELECT failed: %s", PQerrorMessage(conn));
        }
        else
        {
            nFields = PQnfields(res);
            for (i = 0; i < nFields; i++)
                printf("%-15s", PQfname(res, i));
            printf("\n\n");

            for (i = 0; i < PQntuples(res); i++)
            {
                for (j = 0; j < nFields; j++)
                    printf("%-15s", PQgetvalue(res, i, j));
                printf("\n");
            }
        }
        PQclear(res);

        res = PQexec(conn, "SELECT file FROM test WHERE id = 11");
        if (PQresultStatus(res) != PGRES_TUPLES_OK)
        {
            fprintf(stderr, "SELECT failed: %s", PQerrorMessage(conn));
        }
        else
        {

            printf("file to read: %-15s\n", PQgetvalue(res, 0, 0));
            strcpy(oid,PQgetvalue(res, 0, 0));
            printf("oid: %-15s\n", oid);
        }
        PQclear(res);

        OID = (Oid) strtoul(oid,NULL,10);
        OID = 27570;
        printf("OID: %d\n", OID);

        buff =  malloc(2048);

        // MEF il faut ouvrir une transaction pout utiliser lo_open lo_read 
        res = PQexec(conn, "begin");
        PQclear(res);

        fd = lo_open(conn, OID ,INV_READ|INV_WRITE);
        printf("fd: %d\n",fd);

        i = lo_tell(conn,fd);
        printf("%d\n",i);
        i = lo_lseek(conn,fd, 0, SEEK_SET);
        printf("%d\n",i);
        i = lo_read(conn,fd, buff, 500);
        printf("%d\n",i);
        printf("%s\n",buff);

        i = lo_close(conn,fd);
        printf("%d\n",i);

        res = PQexec(conn, "end");
        PQclear(res);

        PQfinish(conn);
    }
}

/**
 * \fn int charger(char * file, char ** buff)
 * \brief chargement d'un fichier dans un buffer
 * 
 * le buffer est malloc dans la fontion devra etre libere
 * 
 * \return taille du fichier
 */
int charger(char *file, char **buff)
{
    FILE *fd = fopen(file, "r");
    if (!fd)
    {
        printf("error fd\n");
        return -1;
    }
    // may be replace by fstat() and stat.st_size
    fseek(fd, 0, SEEK_END);
    int size = ftell(fd);
    fseek(fd, 0, SEEK_SET);
    *buff = malloc(size + 1);
    fread(*buff, 1, size, fd);
    fclose(fd);
    return size;
}