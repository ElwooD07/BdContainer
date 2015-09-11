1. В svn-е не случайно сделана структура trunk, branches, tags
В транке хранится текущася версия проекта. Т.е. то что сейчас лежит в корне должно быть сложено
отдельно в транк, создайте отдельную папочку для документации.
Папка branches существует для реализации разработчиками разных фич одновременно и последующего
мержа в транк. 
В tags складываются версии которые отправлены в продакшн (вам это не понадобится).
Сейчас вам достаточно использовать только транк.

2. Там я скинул файлик proj_structure, сделайте для солюшена соответствующую структуру.
Если не получается закоммитить какие-то файлы, позвоните мне во вторник - я подойду - помогу.
В любом случае позвоние - расскажете о статусе проекта. Я не вижу новых коммитов.

3. Просмотрите замечания в документах и ниже в коде. Я скинул пример интерфейса - ознакомьтесь, сделайте ближе к нему.
Можете разнести реализацию как у вас сейчас для файли и каталога - как вам удобнее, это написан общий интерфейс для
древовидного объекта файловой системы.

//#include <map>
//#include <string>
#include <list>

namespace DBContainer
{

	/*
#ifndef DLL_EXPORT
#define DLL_EXPORT __declspec(dllexport)
#endif
	*/

#ifndef DBCONTAINER

#define EMPTY 0
#define SUCCESS 0
#define ERR_CONT_NOT_FOUND 1
#define ERR_CONT_CANT_OPEN 2
#define SQL_WRONG_QUERY 3

#define DBCONTAINER
#endif

int test(const char * path);

class IContainerElement;

class Container;

class IContainerElement
{
protected:
	Container * owner;
	char * path;

	friend int bindElement(IContainerElement &element, Container * owner);

public:
	int virtual Open(const char * path) = 0;
	int virtual Move(const char * new_path) = 0;
	int virtual Rename(IContainerElement * cont_element, const char * new_name);
	int virtual Delete() = 0;

	// нужно наоборот вызывать из деструктора close
	// и так как это интерфейсы здесь не должно быть реализации, вся реализация должна быть в дргих классах
	// path лучше сделать std::wstring
	// Container нужно завернуть в std::auto_ptr, тогда память будет чиститься автоматические.
	int virtual Close() { this->~IContainerElement(); }

	virtual ~IContainerElement()
	{
		delete owner;
		delete path;
	}
};

class ContainerFile: public IContainerElement
{
public:
	// from IContainerElement
		int virtual Open(const char * path);
		int virtual Move(const char * new_path);
		int virtual Delete();
	//
	ContainerFile(Container * owner, const char * path);
	int Copy(const char * new_path);
	int GetNextData(char * buffer, size_t buffer_size, size_t was_read);
	int WriteNextData(char * data, size_t buffer_size, size_t was_written);
	int Rewind();
	int Truncate();
};

class ContainerFolder: public IContainerElement
{
public:
	// from IContainerElement
		int virtual Open(const char * path);
		int virtual Move(const char * new_path);
		int virtual Delete();
	//
	ContainerFolder(Container * owner, const char * path);
	/*
	DLL_EXPORT map <string,int> GetSoursers(char* FolderName); 
	*/
	int AddNested(const char * folder_name);
	int Copy(const char * new_path, bool copy_nested = false);
	ContainerFile * GetFile(const char * name);
	ContainerFolder * GetFolder(const char * path);
	ContainerFile * GetFirstFile();
	ContainerFile * GetNextFile();
	ContainerFile * GetFirstFolder();
	ContainerFolder * GetNextFolder();
	/*
	int Delete(char * folder_name);
	int DeleteFile(char* FileName) - óäàëÿåò ôàéë èç êîíòåéíåðà ïî çàäàííîìó èìåíè 
	*/
};

class Container
{
	char * db_path; // Path to the database
	char * bin_path; // Path to the binary file with the files data

	std::list<ContainerFile *> files_list;
	std::list<ContainerFolder *> folders_list;

	int lastErrNo;
public:
	
	Container(const char * dbpath, bool create = false);
	int Disconnect();
	int Reconnect(const char * dbpath);

	// Working with the entire files and folders
	IContainerElement * OpenElement(const char * path);
	bool Exists(const char *obj_name);
	ContainerFile * GetFile(const char * path);
	ContainerFolder * GetFOlder(const char * path);
	int ImportFile(const char * existing_file_name, const char * new_file_name);
	int ExportFile(ContainerFile * existing_file, const char * new_file_name);
	int ImportFolder(const char * existing_folder_path, const char * new_folder_path, bool copy_nested = false);
	int ExportFolder(ContainerFolder * existing_folder, const char * new_folder_path, bool copy_nested = false);
	int Remove(IContainerElement * object);

	int Break();
	int LastErrNo();
	char * LastErrDescription();

// !!!!!!!!!!!!This is for testing!!!!!!!!!!!!
	void test();
};

// Binding an element to the container // 
int bindElement(IContainerElement &element, Container * owner);

// Create empty base
int CreateDB(const char * path); //+

// Open/Close
Container Connect(const char * dbpath, bool create = false);
int Disconnect(Container container);

} // namespace DBContainer