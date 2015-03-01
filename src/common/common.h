
#pragma once
#include "include/v8.h"
#include <fstream>
#include <vector>

using namespace v8;
using namespace std;

enum eScriptExecResult {
    eSCRIPT_ERROR_UNKNOWN = 0,
    eSCRIPT_ERROR_NOT_FOUND,
    eSCRIPT_ERROR_EMPTY_SOURCE,
    eSCRIPT_ERROR_COMPILE_FAILED,
    eSCRIPT_ERROR_NONE,
    eSCRIPT_ERROR_COUNT
};

string fileToString(const string &fileName)
{
    ifstream ifs(fileName.c_str(), ios::in | ios::binary | ios::ate);

    if(ifs.fail()){
        printf("File dos not exist! %s\n", fileName.c_str());
        return "";
    }

    ifstream::pos_type fileSize = ifs.tellg();
    ifs.seekg(0, ios::beg);

    vector<char> bytes(fileSize);
    ifs.read(&bytes[0], fileSize);

    return string(&bytes[0], fileSize);
}

    /* Function to print errors to the console */
void reportException(Isolate* isolate,
                     TryCatch* try_catch ) 
{
    Locker lock(isolate);
    HandleScope handle_scope(isolate);

    //Get a string from the error message and exception detail
    String::Utf8Value exception( try_catch->Exception() );
    Handle<Message> message = try_catch->Message();

    //This error has no message
    if (message.IsEmpty()) 
    {
        printf("%s\n" , *exception );
        return;
    }

    char ex[1024];
    String::Utf8Value filename( message->GetScriptResourceName() );
    int linenum = message->GetLineNumber();

    // Print (filename):(line number): (message).
    sprintf( ex , "%s:%i: %s\n", *filename , linenum , *exception );
    printf("%s", ex);

    // Print line of source code.
    String::Utf8Value sourceline( message->GetSourceLine() );
    printf( "%s\n", *sourceline );

}

/*  readFile from the v8 samples. 
    Returns a v8::String from a file. */
const char* readFile(Isolate* isolate, const string str ) 
{
    return fileToString(str).c_str();
}


    /* Execute a specific piece of text in the execution context specified */
bool executeString(Isolate* isolate,
                   const Handle<Context> &context,
                   Local<String> source)
{
    HandleScope handle_scope(isolate);

    if (source->Length() == 0) return false;

    //Switch to the context we want to execute in
    Context::Scope context_scope(context);

    TryCatch try_catch(isolate);

    // Compile the source code.
    Local<Script> script = Script::Compile(source);

    //If the script is empty, there were compile errors.
    if ( script.IsEmpty() )
    {
        reportException(isolate, &try_catch );
        return false;
    } else {
        //So if compilation succeeds, execute it.
        Handle<Value> result = script->Run();

        //If the results are empty, there was a runtime
        //error, so we can report these errors.
        if ( result.IsEmpty() ) 
        {
            reportException(isolate, &try_catch );
            return false;
        }  else {
            //If there is a result, print it to the console
            if ( !result->IsUndefined() && !result.IsEmpty() ) 
            {
                //Convert the results to string
                String::Utf8Value utf8(result);
                printf("%s\n", *utf8);
            }
            //Done
            return true;
        }
    }
}


    /* Execute the script by filename in the execution context specified */
eScriptExecResult executeScript(Isolate* isolate,
                               Local<Context> context,
                               string filename)
{

    //The source code of this file.
    Local<String> source = String::NewFromUtf8(isolate, readFile(isolate, filename));

    //No data in the file.
    if( source.IsEmpty() ) return eSCRIPT_ERROR_EMPTY_SOURCE;

    //Return compilation error
    if ( !executeString(isolate, context, source)) return eSCRIPT_ERROR_COMPILE_FAILED;

    //Succesfully executed
    return eSCRIPT_ERROR_NONE;
}