#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <ctime>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;
using namespace chrono;

void generarArchivo(const string& nombre,long cantidad)
{
    ofstream out(nombre, ios::binary);

    srand(time(NULL));

    for(long i=0;i<cantidad;i++)
    {
        int num = rand();
        out.write((char*)&num,sizeof(int));
    }

    out.close();
}

bool verificarOrdenado(const vector<int>& datos)
{
    for(size_t i=1;i<datos.size();i++)
    {
        if(datos[i-1] > datos[i])
            return false;
    }

    return true;
}

void ordenarMemoria(const string& archivo)
{
    cout << "\n===== ORDENAMIENTO EN MEMORIA =====\n";

    auto inicioTotal = high_resolution_clock::now();

    int fd = open(archivo.c_str(), O_RDWR);

    struct stat st;
    fstat(fd,&st);

    size_t tamArchivo = st.st_size;

    auto inicioMap = high_resolution_clock::now();

    void* map = mmap(
        NULL,
        tamArchivo,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        fd,
        0);

    auto finMap = high_resolution_clock::now();

    int* datos = (int*)map;

    size_t cantidad = tamArchivo / sizeof(int);

    auto inicioSort = high_resolution_clock::now();

    sort(datos, datos + cantidad);

    auto finSort = high_resolution_clock::now();

    msync(map,tamArchivo,MS_SYNC);

    auto finTotal = high_resolution_clock::now();

    cout << "Numeros: " << cantidad << endl;

    cout << "Tiempo mapeo: "
         << duration<double>(finMap-inicioMap).count()
         << " s\n";

    cout << "Tiempo ordenamiento: "
         << duration<double>(finSort-inicioSort).count()
         << " s\n";

    cout << "Tiempo total: "
         << duration<double>(finTotal-inicioTotal).count()
         << " s\n";

    munmap(map,tamArchivo);
    close(fd);
}

void ordenarDisco(const string& archivo,long tamBloque)
{
    cout << "\n===== ORDENAMIENTO EN DISCO =====\n";

    auto inicioTotal = high_resolution_clock::now();

    ifstream in(archivo, ios::binary);

    vector<string> temporales;

    int bloqueNumero=0;

    auto inicioLectura = high_resolution_clock::now();

    while(true)
    {
        vector<int> bloque(tamBloque);

        in.read((char*)bloque.data(),
                tamBloque*sizeof(int));

        long leidos =
        in.gcount()/sizeof(int);

        if(leidos==0)
            break;

        bloque.resize(leidos);

        sort(bloque.begin(), bloque.end());

        string temp =
        "temp_"+to_string(bloqueNumero++)+".bin";

        temporales.push_back(temp);

        ofstream out(temp, ios::binary);

        out.write(
            (char*)bloque.data(),
            leidos*sizeof(int));

        out.close();
    }

    auto finLectura = high_resolution_clock::now();

    in.close();

    auto finTotal = high_resolution_clock::now();

    cout << "Bloques creados: "
         << temporales.size()
         << endl;

    cout << "Tamano bloque: "
         << tamBloque
         << " numeros\n";

    cout << "Tiempo bloques: "
         << duration<double>(
            finLectura-inicioLectura).count()
         << " s\n";

    cout << "Tiempo total: "
         << duration<double>(
            finTotal-inicioTotal).count()
         << " s\n";

    cout << "(Version simplificada sin merge final)\n";
}

int main(int argc,char* argv[])
{
    if(argc != 2)
    {
        cout
        << "Uso: "
        << argv[0]
        << " cantidad_numeros\n";

        return 1;
    }

    long cantidad = atol(argv[1]);

    string archivo = "datos.bin";

    cout << "Generando archivo...\n";

    generarArchivo(
        archivo,
        cantidad);

    cout << "Numeros generados: "
         << cantidad
         << endl;

    double mb =
    (cantidad*sizeof(int))
    /(1024.0*1024.0);

    cout << "Tamano aproximado: "
         << mb
         << " MB\n";

    ordenarMemoria(archivo);

    ordenarDisco(
        archivo,
        100000);

    return 0;
}
