#include <Windows.h>
#include "jni.h"

DWORD WINAPI loadjre(LPVOID lpParam) {
	MessageBoxA(NULL, "Injection Successful!", "RCT2MP", MB_OK);

	HKEY jreKey;
	RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\JavaSoft\\Java Runtime Environment", 0, KEY_READ, &jreKey);
	
	char jreVersion[8] = {0};	
	DWORD jreVersionSize = sizeof(jreVersion);
	RegQueryValueExA(jreKey, "CurrentVersion", 0, NULL, (LPBYTE) jreVersion, &jreVersionSize);

	MessageBoxA(NULL, jreVersion, "RCT2MP", MB_OK);

	HKEY jreSubKey;
	RegOpenKeyExA(jreKey, jreVersion, 0, KEY_READ, &jreSubKey);

	char jreRuntimeLib[MAX_PATH] = {0};
	DWORD jreRuntimeLibSize = sizeof(jreRuntimeLib);
	RegQueryValueExA(jreSubKey, "RuntimeLib", 0, NULL, (LPBYTE) jreRuntimeLib, &jreRuntimeLibSize);

	RegCloseKey(jreSubKey);
	RegCloseKey(jreKey);

	MessageBoxA(NULL, jreRuntimeLib, "RCT2MP", MB_OK);

	typedef jint (JNICALL CreateJavaVM_t)(JavaVM **pvm, JNIEnv **env, void *args);
	HMODULE jreHandle = LoadLibraryA(jreRuntimeLib);

	CreateJavaVM_t *CreateJavaVM = (CreateJavaVM_t *) GetProcAddress(jreHandle, "JNI_CreateJavaVM");

	JavaVM *jvm;
	JNIEnv *env;
	JavaVMInitArgs vm_args;

	vm_args.version = JNI_VERSION_1_6;
    vm_args.nOptions = 0;
    vm_args.options = NULL;
	vm_args.ignoreUnrecognized = false;

	CreateJavaVM(&jvm, &env, &vm_args);

	jclass JOptionPane = env->FindClass("javax/swing/JOptionPane");
	jmethodID JOptionPane_showMessageDialog = env->GetStaticMethodID(JOptionPane, "showMessageDialog", "(Ljava/awt/Component;Ljava/lang/Object;)V");

	jstring message = env->NewStringUTF("Hello World!");
	env->CallStaticVoidMethod(JOptionPane, JOptionPane_showMessageDialog, NULL, message);

	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD reason, LPVOID reserved) {
	switch(reason) {
		case DLL_PROCESS_ATTACH:
			{
				DisableThreadLibraryCalls(hInstance);
				CreateThread(NULL, 0, loadjre, NULL, 0, NULL);
			}
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			// Should never happen
		case DLL_PROCESS_DETACH:
			// Ignore
			break;
    }

    return TRUE;
}