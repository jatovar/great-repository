// fstest.cc 
//	Simple test routines for the file system.  
//
//	We implement:
//	   Copy -- copy a file from UNIX to Nachos
//	   Print -- cat the contents of a Nachos file 
//	   Perftest -- a stress test for the Nachos file system
//		read and write a really large file in tiny chunks
//		(won't work on baseline system!)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "utility.h"
#include "filesys.h"
#include "system.h"
#include "thread.h"
#include "disk.h"
#include "stats.h"

#define TransferSize 	10 	// make it small, just to be difficult

char* filtraArgumentos(int argc, char** argv, char* nomUsuario);
char** procesaComando(int& argc);
void eliminaArgumentos(int argc, char** argv);

//----------------------------------------------------------------------
// Copy
// 	Copy the contents of the UNIX file "from" to the Nachos file "to"
//----------------------------------------------------------------------

void
Copy(char *from, char *to)
{
    FILE *fp;
    OpenFile* openFile;
    int amountRead, fileLength;
    char *buffer;

// Open UNIX file
    if ((fp = fopen(from, "r")) == NULL) {	 
	printf("Copy: couldn't open input file %s\n", from);
	return;
    }

// Figure out length of UNIX file
    fseek(fp, 0, 2);		
    fileLength = ftell(fp);
    fseek(fp, 0, 0);

// Create a Nachos file of the same length
    DEBUG('f', "Copying file %s, size %d, to file %s\n", from, fileLength, to);
    if (!fileSystem->Create(to, fileLength)) {	 // Create Nachos file
	printf("Copy: couldn't create output file %s\n", to);
	fclose(fp);
	return;
    }
    
    openFile = fileSystem->Open(to);
    ASSERT(openFile != NULL);
    
// Copy the data in TransferSize chunks
    buffer = new char[TransferSize];
    while ((amountRead = fread(buffer, sizeof(char), TransferSize, fp)) > 0)
	openFile->Write(buffer, amountRead);	
    delete [] buffer;

// Close the UNIX and the Nachos files
    delete openFile;
    fclose(fp);
}

//----------------------------------------------------------------------
// Print
// 	Print the contents of the Nachos file "name".
//----------------------------------------------------------------------

void
Print(char *name)
{
    OpenFile *openFile;    
    int i, amountRead;
    char *buffer;

    if ((openFile = fileSystem->Open(name)) == NULL) {
	printf("Print: unable to open file %s\n", name);
	return;
    }
    
    buffer = new char[TransferSize];
    while ((amountRead = openFile->Read(buffer, TransferSize)) > 0)
	for (i = 0; i < amountRead; i++)
	    printf("%c", buffer[i]);
    delete [] buffer;

    delete openFile;		// close the Nachos file
    return;
}

//----------------------------------------------------------------------
// PerformanceTest
// 	Stress the Nachos file system by creating a large file, writing
//	it out a bit at a time, reading it back a bit at a time, and then
//	deleting the file.
//
//	Implemented as three separate routines:
//	  FileWrite -- write the file
//	  FileRead -- read the file
//	  PerformanceTest -- overall control, and print out performance #'s
//----------------------------------------------------------------------

#define FileName 	"TestFile"
#define Contents 	"1234567890"
#define ContentSize 	strlen(Contents)
#define FileSize 	((int)(ContentSize * 5000))

static void 
FileWrite()
{
    OpenFile *openFile;    
    int i, numBytes;

    printf("Sequential write of %d byte file, in %d byte chunks\n", 
	FileSize, ContentSize);
    if (!fileSystem->Create(FileName, 0)) {
      printf("Perf test: can't create %s\n", FileName);
      return;
    }
    openFile = fileSystem->Open(FileName);
    if (openFile == NULL) {
	printf("Perf test: unable to open %s\n", FileName);
	return;
    }
    for (i = 0; i < FileSize; i += ContentSize) {
        numBytes = openFile->Write(Contents, ContentSize);
	if (numBytes < 10) {
	    printf("Perf test: unable to write %s\n", FileName);
	    delete openFile;
	    return;
	}
    }
    delete openFile;	// close file
}

static void 
FileRead()
{
    OpenFile *openFile;    
    char *buffer = new char[ContentSize];
    int i, numBytes;

    printf("Sequential read of %d byte file, in %d byte chunks\n", 
	FileSize, ContentSize);

    if ((openFile = fileSystem->Open(FileName)) == NULL) {
	printf("Perf test: unable to open file %s\n", FileName);
	delete [] buffer;
	return;
    }
    for (i = 0; i < FileSize; i += ContentSize) {
        numBytes = openFile->Read(buffer, ContentSize);
	if ((numBytes < 10) || strncmp(buffer, Contents, ContentSize)) {
	    printf("Perf test: unable to read %s\n", FileName);
	    delete openFile;
	    delete [] buffer;
	    return;
	}
    }
    delete [] buffer;
    delete openFile;	// close file
}

void
PerformanceTest()
{
    printf("Starting file system performance test:\n");
    stats->Print();
    FileWrite();
    FileRead();
    if (!fileSystem->Remove(FileName)) {
      printf("Perf test: unable to remove %s\n", FileName);
      return;
    }
    stats->Print();
}

void mkuser(char* nombre){	//Comando mkuser
	if (fileSystem->CreaUsuario(nombre, 0)){
		if (!fileSystem->CreaUFD(nombre, 0))
			printf ("No se pudo crear UFD\n");
	}else
		printf ("No se pudo crear el usuario\n");
}

void rmuser(char* nombre){	//Comando rmuser
	if (fileSystem->RemueveArchivos(nombre)){
		if (fileSystem->RemueveUFD(nombre)){
			/*if (!fileSystem->RemueveUsuario(nombre))
				printf ("No se pudo borrar el directorio de usuario\n");*/
		}else
			printf ("Ocurrio un problema al borrar el directorio UFD\n");
	}else
		printf ("Ocurrio un problema al borrar los archivos\n");
}

void format(){	//Comando format
	if (fileSystem)
		delete fileSystem;
	fileSystem = new FileSystem(true);
}

void copy(char* nomUsuario, char* desde, char* hasta){	//Comando copy
	FILE *fp;
	OpenFile* openFile = NULL;
	int amountRead, fileLength;
	char *buffer;
	
	if ((fp = fopen(desde, "r")) == NULL) {	 
		printf("copy: no se pudo leer el archivo %s\n", desde);
		return;
	}
	fseek(fp, 0, 2);		
	fileLength = ftell(fp);
	fseek(fp, 0, 0);
	DEBUG('f', "Copying file %s, size %d, to file %s\n", desde, fileLength, hasta);
	if (!fileSystem->CreaArchivo(nomUsuario, hasta, fileLength)) {	 // Create Nachos file
		printf("copy: no se pudo crear el archivo %s\n", hasta);
		fclose(fp);
		return;
	}
	openFile = fileSystem->Abrir(nomUsuario, hasta);
	ASSERT(openFile != NULL);
	buffer = new char[TransferSize];
	while ((amountRead = fread(buffer, sizeof(char), TransferSize, fp)) > 0)
		openFile->Write(buffer, amountRead);
	delete [] buffer;
	delete openFile;
	fclose(fp);
}

bool changeDir(char* nomUsuario){	//Comando cd
	return fileSystem->ExisteUsuario(nomUsuario);
}

void Archivo(char *user,char *nomArch){
	fileSystem->CreaArchivo(user,nomArch,50);
}

void archivos(char* nomUsuario){ //Comando ls
	fileSystem->listaArchivos(nomUsuario);
}

void manual(char* comando){	//Comando manual
	printf ("Manual: %s\n", comando);
	if (!strcmp(comando, "format")){
		printf ("\tUtilidad de NachOS para formatear el disco\n");
		printf ("\tNo se necesita ser usuario root para formatear el disco\n");
		printf ("\tSINTAXIS: format\n");
	}
	if (!strcmp(comando, "mkuser")){
		printf ("\tUtilidad de NachOS para crear un usuario\n");
		printf ("\tAl crear un usuario, este se registra en el MFD (Master File Directory),\n");
		printf ("\ty a su vez se crea un directorio personal vacio UFD (User File Directory)\n");
		printf ("\tSe puede crear un usuario desde cualquier directorio\n");
		printf ("\tSINTAXIS: mkuser [usuario]\n");
	}
	if (!strcmp(comando, "rmuser")){
		printf ("\tUtilidad de NachOS para borrar un usuario\n");
		printf ("\tEste comando borra un usuario de manera total del sistema, empezando desde los archivos\n");
		printf ("\tpersonales, despues se borra el UFD y al final se borra el registro del MFD\n");
		printf ("\tSe puede borrar un usuario desde cualquier directorio\n");
		printf ("\tEl usuario debe de estar registrado en el MFD\n");
		printf ("\tSINTAXIS: rmuser [usuario]\n");
	}
	if (!strcmp(comando, "cd")){
		printf ("\tUtilidad clon-UNIX para acceder a un directorio\n");
		printf ("\tPermite navegar entre directorios del primer nivel\n");
		printf ("\tEl usuario debe de estar registrado en el MFD\n");
		printf ("\tSINTAXIS: cd [usuario]\n");
	}
	if (!strcmp(comando, "list")){
		printf ("\tUtilidad clon-UNIX para listar el nombre de todo lo contenido en MFD y UFD\n");
		printf ("\tPermite listar los nombres de todos los archivos por usuario\n");
		printf ("\tSINTAXIS: list\n");
	}
	if (!strcmp(comando, "copy")){
		printf ("\tUtilidad clon-UNIX para copiar un archivo desde la arquitectura real hasta NachOS\n");
		printf ("\tPermite copiar un archivo de la arquitectura host, al disco duro virtual de NachOS\n");
		printf ("\tSINTAXIS: copy [RutaFisicaEnHost] [UsuarioActual]\n");
	}
	if (!strcmp(comando, "exit")){
		printf ("\tUti	lidad de NachOS para terminar la ejecucion del Shell\n");
		printf ("\tTermina la ejecucion de la instancia de NachOS actual\n");
		printf ("\tSINTAXIS: exit\n");
	}
	if (!strcmp(comando, "man")){
		printf ("\tUtilidad clon-UNIX para desplegar la ayuda de los comandos\n");
		printf ("\tMuestra informacion relacionada con cada comando implementado\n");
		printf ("\tSINTAXIS: manual [comando]\n");
	}
	if (!strcmp(comando, "help")){
		printf ("\tUtilidad de NachOS para desplegar la lista de comandos\n");
		printf ("\tMuestra una lista con todos los comandos disponibles\n");
		printf ("\tSINTAXIS: help\n");
	}
}

void help(){	//Comando help
	printf ("Comandos disponibles:\nformat\nmkuser\nrmuser\ncd\nlist\ncopy\nmanual\nhelp\nexit\n");
}

char* filtraArgumentos(int argc, char** argv, char* nomUsuario){	//Se encarga de interpretar los comandos dados en el shell
	char* cmd	 = NULL;
	FILE *arch;	

	if (argv){
		if (!strcmp(*(argv+0), "format")){
			ASSERT(argc > 0 && argc <= 1);
			format();
		}
		if (!strcmp(*(argv+0), "mkuser")){
			ASSERT(argc > 1 && argc <= 2);
			mkuser(*(argv+1));
		}
		if (!strcmp(*(argv+0), "rmuser")){
			ASSERT(argc > 1 && argc <= 2);
			rmuser(*(argv+1));
		}
		if (!strcmp(*(argv+0), "cd")){
			ASSERT(argc > 1 && argc <= 2);
			if (changeDir(*(argv+1))){
				cmd = new char[strlen(*(argv+1)) + 2 + 1];	//Prefijo cd, caracter nulo
				strcpy (cmd, "cd");
				strcat (cmd, *(argv + 1));
				*(cmd + strlen(cmd)) = '\x0';
			} else {
				if(strcmp(*(argv+1),"/") == 0){
					cmd = new char[strlen("root") + 1];
					strcpy (cmd, "root");
					printf("cabia a root\n");
				} else {
					printf ("Usuario no existente\n");
				}
			}
		}
		if (!strcmp(*(argv+0), "list")){
			ASSERT(argc > 0 && argc <= 1);
			fileSystem->List();
		}
		if (!strcmp(*(argv+0), "copy")){
			arch = fopen("hola.txt","w");
			fclose(arch);
			ASSERT(argc > 2 && argc <= 3);
			if (changeDir(nomUsuario))
				copy(nomUsuario, *(argv+1), *(argv+2));
			else
				printf ("No se puede escribir dentro de root\n");
		}
		if (!strcmp(*(argv+0), "exit")){
			ASSERT(argc > 0 && argc <= 1);
			cmd = new char[strlen(*(argv+0)) + 1];
			strcpy (cmd, *(argv+0));
		}
		if (!strcmp(*(argv+0), "man")){
			ASSERT(argc > 1 && argc <= 2);
			manual(*(argv+1));		
		}
		if (!strcmp(*(argv+0), "help")){
			ASSERT(argc > 0 && argc <= 1);
			help();
		}
		if (!strcmp(*(argv), "mkfile")){
			printf("Nuevo Archivo\n");
			ASSERT(argc > 0 && argc <= 2);
			Archivo(nomUsuario,*(argv+1));
		}
		if (!strcmp(*(argv+0), "ls")){
			ASSERT(argc > 0 && argc <= 1);
			archivos(nomUsuario);
		}
	}
	else{
		cmd = new char[strlen("exit") + 1];
		strcpy (cmd, "exit");
	}
	return cmd;
}

void Shell(){
	char **argv;			//Argumentos en especifico
	int argc = 0;			//Cuenta los argumentos del comando dado en cmd
	char usuario[64] = "root";	//Usuario actual... Tomado del MFD
	char rutaUsuario[64] = "";
	char* res = NULL;
	bool salida = false;

	strcpy(rutaUsuario, usuario);
	strcat(rutaUsuario, "@sob:~/ ");
	printf("NachOS Shell\nPuedes consultar ayuda tecleando el comando help\n");
	do{
		printf("%s", rutaUsuario);		//Nombre de usuario
		argv = procesaComando(argc);
		res = filtraArgumentos(argc, argv, usuario);
		eliminaArgumentos(argc, argv);	
		if (res){
			if (*(res) == 'c' && *(res + 1) == 'd'){
				*(usuario) = '\x0';
				strcpy(usuario, res + 2);
				strcpy(rutaUsuario, usuario);
				strcat(rutaUsuario, "@sob:~/ ");
			}
			if(!strcmp(res,"root")){
				strcpy(usuario, res);
				strcpy(rutaUsuario, usuario);
				strcat(rutaUsuario, "@sob:~/ ");
			}
			if (!strcmp(res, "exit"))
				salida = true;
			delete [] res;
		}
	}while(!salida);
}

char** procesaComando(int& argc){	//Se encarga de separa las cadenas una por una
	char **argv = 0;	//Cadena que sera regreseda con todo el comando particionado
	char cmd[64] = "";	//Toda la cadena con todo el comando escrito en una sola linea
	char argTmp[32] = "";
	int count;

	fgets(cmd, 64, stdin);		//Toma de Comando
	cmd[strlen(cmd) - 1] = '\x0';	//Quita salto de linea
	argc = 1;

	for (int i = 0; i < strlen(cmd); i++)	//Cuenta todas las partes del comando
		if (*(cmd + i) == 0x20)
			argc++;
	argv = new char*[argc];			//Crea un arreglo de cadenas para cada argumento
	for (int i = 0, arg = 0; i < argc; i++){
		count = 0;
		for (int j = 0; j < sizeof(argTmp) && *(cmd + arg) != 0x20 && *(cmd + arg) != '\x0'; j++){
			*(argTmp+j) = *(cmd+arg);	//Considera que cada espacio es el termino de un argumento
			arg++;
			count++;
		}
		arg++;	//omite la posicion del espacio el espacio
		*(argv+i) = new char[sizeof(argTmp)];
		*(argTmp + count) = '\x0';
		strcpy(*(argv+i), argTmp);
		*(argTmp) = '\x0';	//Se limpia cadena temporal
	}
	return argv;
}

void eliminaArgumentos(int argc, char** argv){	//Libera el arreglo de cadenas generado por procesaComando
	for (int i = 0; i < argc; i++)
		delete [] *(argv+i);
	delete [] argv;
}













