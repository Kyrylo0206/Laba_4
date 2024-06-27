#include <iostream>
#include <stack>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <windows.h>

using namespace std;

#define BUFFER_SIZE 1024
#define INITIAL_CAPACITY 10

class Caesar {
public:
    Caesar(const char* lllllb4);
    ~Caesar();
    char* encrypt(char* text, int key);
    char* decrypt(char* text, int key);

private:
    HMODULE hModule;
    typedef char* (__cdecl* EncryptFunc)(char*, int);
    typedef char* (__cdecl* DecryptFunc)(char*, int);
    EncryptFunc encryptFunc;
    DecryptFunc decryptFunc;
};

Caesar::Caesar(const char* lllllb4) {
    hModule = LoadLibrary(TEXT("lllllb4.dll"));
    if (!hModule) {
        throw runtime_error("Library not found");
    }
    encryptFunc = (EncryptFunc)GetProcAddress(hModule, "encrypt");
    decryptFunc = (DecryptFunc)GetProcAddress(hModule, "decrypt");
    if (!encryptFunc || !decryptFunc) {
        FreeLibrary(hModule);
        throw runtime_error("Failed to get address of encrypt or decrypt function");
    }
}

Caesar::~Caesar() {
    if (hModule) {
        FreeLibrary(hModule);
    }
}

char* Caesar::encrypt(char* text, int key) {
    return encryptFunc(text, key);
}

char* Caesar::decrypt(char* text, int key) {
    return decryptFunc(text, key);
}

class TextStorage {
public:
    TextStorage();
    ~TextStorage();
    void appendText(const char* newText);
    void newLine();
    void saveToFile(const char* filename);
    void loadFromFile(const char* filename);
    void printText();
    void insertText(int line, int index, const char* str);
    void deleteText(int line, int index, int length);
    void searchText(const char* str);
    void clearConsole();
    void freeTextStorage();
    char* getText(int line);
    int getTotalLines() const;

private:
    char** text;
    int capacity;
    int totalLines;

    void ensureCapacity();
};

TextStorage::TextStorage() : capacity(INITIAL_CAPACITY), totalLines(0) {
    text = (char**)malloc(capacity * sizeof(char*));
    for (int i = 0; i < capacity; ++i) {
        text[i] = (char*)malloc(BUFFER_SIZE * sizeof(char));
        text[i][0] = '\0';
    }
}

TextStorage::~TextStorage() {
    freeTextStorage();
}

void TextStorage::appendText(const char* newText) {
    ensureCapacity();
    if (newText != nullptr) {
        strcpy_s(text[totalLines], BUFFER_SIZE, newText);
        totalLines++;
    }
}

void TextStorage::newLine() {
    ensureCapacity();
    text[totalLines][0] = '\0';
    totalLines++;
}

void TextStorage::saveToFile(const char* filename) {
    FILE* file;
    errno_t err = fopen_s(&file, filename, "w");
    if (err != 0) {
        cerr << "Could not open file for writing.\n";
        return;
    }
    for (int i = 0; i < totalLines; i++) {
        fprintf(file, "%s\n", text[i]);
    }
    fclose(file);
}

void TextStorage::loadFromFile(const char* filename) {
    FILE* file;
    errno_t err = fopen_s(&file, filename, "r");
    if (err != 0) {
        cerr << "Error opening file for reading.\n";
        return;
    }
    char line[BUFFER_SIZE];
    while (fgets(line, BUFFER_SIZE, file)) {
        line[strcspn(line, "\n")] = 0;
        appendText(line);
    }
    fclose(file);
}

void TextStorage::printText() {
    for (int i = 0; i < totalLines; i++) {
        cout << text[i] << endl;
    }
}

void TextStorage::insertText(int line, int index, const char* str) {
    if (line < 0 || line >= totalLines) {
        cerr << "Invalid line number.\n";
        return;
    }
    if (index < 0 || index > strlen(text[line])) {
        cerr << "Invalid index.\n";
        return;
    }
    char buffer[BUFFER_SIZE];
    strncpy_s(buffer, text[line], index);
    buffer[index] = '\0';
    strcat_s(buffer, str);
    strcat_s(buffer, &text[line][index]);
    strcpy_s(text[line], BUFFER_SIZE, buffer);
}

void TextStorage::deleteText(int line, int index, int length) {
    if (line < 0 || line >= totalLines) {
        cerr << "Invalid line number.\n";
        return;
    }
    if (index < 0 || index > strlen(text[line])) {
        cerr << "Invalid index.\n";
        return;
    }
    memmove(&text[line][index], &text[line][index + length], strlen(&text[line][index + length]) + 1);
}

void TextStorage::searchText(const char* str) {
    for (int i = 0; i < totalLines; i++) {
        char* position = strstr(text[i], str);
        while (position != nullptr) {
            int index = position - text[i];
            cout << "Found on line " << i << ", index " << index << endl;
            position = strstr(position + 1, str);
        }
    }
}

void TextStorage::clearConsole() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void TextStorage::freeTextStorage() {
    for (int i = 0; i < capacity; i++) {
        free(text[i]);
    }
    free(text);
}

char* TextStorage::getText(int line) {
    if (line < 0 || line >= totalLines) {
        return nullptr;
    }
    return text[line];
}

int TextStorage::getTotalLines() const {
    return totalLines;
}

void TextStorage::ensureCapacity() {
    if (totalLines >= capacity) {
        int newCapacity = capacity + INITIAL_CAPACITY;
        char** newText = (char**)realloc(text, newCapacity * sizeof(char*));
        if (!newText) {
            cerr << "Failed to allocate more memory.\n";
            return;
        }
        for (int i = capacity; i < newCapacity; i++) {
            newText[i] = (char*)malloc(BUFFER_SIZE * sizeof(char));
            newText[i][0] = '\0';
        }
        text = newText;
        capacity = newCapacity;
    }
}

class Command {
public:
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual ~Command() = default;
};

class InsertCommand : public Command {
public:
    InsertCommand(TextStorage* storage, int line, int index, const char* str);
    void execute() override;
    void undo() override;

private:
    TextStorage* storage;
    int line;
    int index;
    char* str;
};

InsertCommand::InsertCommand(TextStorage* storage, int line, int index, const char* str)
        : storage(storage), line(line), index(index) {
    this->str = (char*)malloc((strlen(str) + 1) * sizeof(char));
    strcpy_s(this->str, strlen(str) + 1, str);
}

void InsertCommand::execute() {
    storage->insertText(line, index, str);
}

void InsertCommand::undo() {
    storage->deleteText(line, index, strlen(str));
    free(str);
}

class DeleteCommand : public Command {
public:
    DeleteCommand(TextStorage* storage, int line, int index, int length);
    void execute() override;
    void undo() override;

private:
    TextStorage* storage;
    int line;
    int index;
    int length;
    char* deletedText;
};

DeleteCommand::DeleteCommand(TextStorage* storage, int line, int index, int length)
        : storage(storage), line(line), index(index), length(length) {
    deletedText = nullptr;
}

void DeleteCommand::execute() {
    char* lineText = storage->getText(line);
    if (lineText) {
        deletedText = (char*)malloc((length + 1) * sizeof(char));
        strncpy_s(deletedText, length + 1, &lineText[index], length);
        deletedText[length] = '\0';
        storage->deleteText(line, index, length);
    }
}

void DeleteCommand::undo() {
    if (deletedText) {
        storage->insertText(line, index, deletedText);
        free(deletedText);
    }
}

class CutCommand : public Command {
public:
    CutCommand(TextStorage* storage, int line, int index, int length, char* clipboard);
    void execute() override;
    void undo() override;

private:
    TextStorage* storage;
    int line;
    int index;
    int length;
    char* cutText;
    char* clipboard;
};

CutCommand::CutCommand(TextStorage* storage, int line, int index, int length, char* clipboard)
        : storage(storage), line(line), index(index), length(length), clipboard(clipboard) {
    cutText = nullptr;
}

void CutCommand::execute() {
    char* lineText = storage->getText(line);
    if (lineText) {
        cutText = (char*)malloc((length + 1) * sizeof(char));
        strncpy_s(cutText, length + 1, &lineText[index], length);
        cutText[length] = '\0';
        strcpy_s(clipboard, BUFFER_SIZE, cutText);
        storage->deleteText(line, index, length);
    }
}

void CutCommand::undo() {
    if (cutText) {
        storage->insertText(line, index, cutText);
        free(cutText);
    }
}

class CopyCommand : public Command {
public:
    CopyCommand(TextStorage* storage, int line, int index, int length, char* clipboard);
    void execute() override;
    void undo() override {}

private:
    TextStorage* storage;
    int line;
    int index;
    int length;
    char* clipboard;
};

CopyCommand::CopyCommand(TextStorage* storage, int line, int index, int length, char* clipboard)
        : storage(storage), line(line), index(index), length(length), clipboard(clipboard) {}

void CopyCommand::execute() {
    char* lineText = storage->getText(line);
    if (lineText) {
        char* copiedText = (char*)malloc((length + 1) * sizeof(char));
        strncpy_s(copiedText, length + 1, &lineText[index], length);
        copiedText[length] = '\0';
        strcpy_s(clipboard, BUFFER_SIZE, copiedText);
        free(copiedText);
    }
}

class PasteCommand : public Command {
public:
    PasteCommand(TextStorage* storage, int line, int index, char* clipboard);
    void execute() override;
    void undo() override;

private:
    TextStorage* storage;
    int line;
    int index;
    char* clipboard;
    char* pastedText;
};

PasteCommand::PasteCommand(TextStorage* storage, int line, int index, char* clipboard)
        : storage(storage), line(line), index(index), clipboard(clipboard) {
    pastedText = nullptr;
}

void PasteCommand::execute() {
    if (clipboard[0] != '\0') {
        pastedText = (char*)malloc((strlen(clipboard) + 1) * sizeof(char));
        strcpy_s(pastedText, strlen(clipboard) + 1, clipboard);
        storage->insertText(line, index, pastedText);
    }
}

void PasteCommand::undo() {
    if (pastedText) {
        storage->deleteText(line, index, strlen(pastedText));
        free(pastedText);
    }
}

class InsertReplaceCommand : public Command {
public:
    InsertReplaceCommand(TextStorage* storage, int line, int index, const char* str);
    void execute() override;
    void undo() override;

private:
    TextStorage* storage;
    int line;
    int index;
    char* newText;
    char* oldText;
};

InsertReplaceCommand::InsertReplaceCommand(TextStorage* storage, int line, int index, const char* str)
        : storage(storage), line(line), index(index) {
    newText = (char*)malloc((strlen(str) + 1) * sizeof(char));
    strcpy_s(newText, strlen(str) + 1, str);
    oldText = (char*)malloc((strlen(storage->getText(line)) + 1) * sizeof(char));
    strcpy_s(oldText, strlen(storage->getText(line)) + 1, storage->getText(line));
}

void InsertReplaceCommand::execute() {
    storage->insertText(line, index, newText);
}

void InsertReplaceCommand::undo() {
    strcpy_s(storage->getText(line), BUFFER_SIZE, oldText);
    free(newText);
    free(oldText);
}

class TextEditor {
public:
    TextEditor();
    ~TextEditor();
    void run();

private:
    TextStorage* textStorage;
    Caesar* caesarCipher;
    stack<Command*> undoStack;
    stack<Command*> redoStack;
    char clipboard[BUFFER_SIZE];

    void handleCommand(int command);
    void appendText(const char* newText);
    void newLine();
    void saveToFile(const char* filename);
    void loadFromFile(const char* filename);
    void printText();
    void insertText(int line, int index, const char* str);
    void deleteText(int line, int index, int length);
    void searchText(const char* str);
    void clearConsole();
    void freeTextStorage();
    void undo();
    void redo();
    void cutText(int line, int index, int length);
    void copyText(int line, int index, int length);
    void pasteText(int line, int index);
    void insertReplaceText(int line, int index, const char* str);
    void encryptFile(const char* inputFile, const char* outputFile, int key);
    void decryptFile(const char* inputFile, const char* outputFile, int key);
};

TextEditor::TextEditor() {
    textStorage = new TextStorage();
    caesarCipher = new Caesar("caesar.dll");
    clipboard[0] = '\0';
}

TextEditor::~TextEditor() {
    delete textStorage;
    delete caesarCipher;
    while (!undoStack.empty()) {
        delete undoStack.top();
        undoStack.pop();
    }
    while (!redoStack.empty()) {
        delete redoStack.top();
        redoStack.pop();
    }
}

void TextEditor::run() {
    int user_input;
    char inputBuffer[BUFFER_SIZE];
    int line, index, length, key;
    char filename[BUFFER_SIZE];

    while (true) {
        cout << "\nText Editor Menu:\n"
             << "1. Append Text\n"
             << "2. New Line\n"
             << "3. Save to File\n"
             << "4. Load from File\n"
             << "5. Print Text\n"
             << "6. Insert Text\n"
             << "7. Delete Text\n"
             << "8. Search Text\n"
             << "9. Clear Console\n"
             << "10. Undo\n"
             << "11. Redo\n"
             << "12. Cut Text\n"
             << "13. Copy Text\n"
             << "14. Paste Text\n"
             << "15. Insert with Replacement\n"
             << "16. Encrypt File\n"
             << "17. Decrypt File\n"
             << "0. Exit\n"
             << "Enter command: ";
        cin >> user_input;
        cin.ignore();

        if (user_input == 0) break;

        handleCommand(user_input);
    }
}

void TextEditor::handleCommand(int command) {
    int line, index, length, key;
    char inputBuffer[BUFFER_SIZE];
    char filename[BUFFER_SIZE];

    switch (command) {
        case 1:
            cout << "Enter text to append: ";
            cin.getline(inputBuffer, BUFFER_SIZE);
            appendText(inputBuffer);
            break;
        case 2:
            newLine();
            break;
        case 3:
            cout << "Enter the file name for saving: ";
            cin.getline(filename, BUFFER_SIZE);
            saveToFile(filename);
            break;
        case 4:
            cout << "Enter the file name for loading: ";
            cin.getline(filename, BUFFER_SIZE);
            loadFromFile(filename);
            break;
        case 5:
            printText();
            break;
        case 6:
            cout << "Enter line and index: ";
            cin >> line >> index;
            cin.ignore();
            cout << "Enter text to insert: ";
            cin.getline(inputBuffer, BUFFER_SIZE);
            insertText(line, index, inputBuffer);
            break;
        case 7:
            cout << "Enter text to search: ";
            cin.getline(inputBuffer, BUFFER_SIZE);
            searchText(inputBuffer);
            break;
        case 8:
            cout << "Enter line, index and length: ";
            cin >> line >> index >> length;
            cin.ignore();
            deleteText(line, index, length);
            break;
        case 9:
            clearConsole();
            break;
        case 10:
            undo();
            break;
        case 11:
            redo();
            break;
        case 12:
            cout << "Enter line, index and length for cut: ";
            cin >> line >> index >> length;
            cin.ignore();
            cutText(line, index, length);
            break;
        case 13:
            cout << "Enter line, index and length for copy: ";
            cin >> line >> index >> length;
            cin.ignore();
            copyText(line, index, length);
            break;
        case 14:
            cout << "Enter line and index for paste: ";
            cin >> line >> index;
            cin.ignore();
            pasteText(line, index);
            break;
        case 15:
            cout << "Enter line and index for replacement: ";
            cin >> line >> index;
            cin.ignore();
            cout << "Enter new text: ";
            cin.getline(inputBuffer, BUFFER_SIZE);
            insertReplaceText(line, index, inputBuffer);
            break;
        case 16:
            cout << "Enter input file, output file and key for encryption: ";
            cin >> filename >> inputBuffer >> key;
            cin.ignore();
            encryptFile(filename, inputBuffer, key);
            break;
        case 17:
            cout << "Enter input file, output file and key for decryption: ";
            cin >> filename >> inputBuffer >> key;
            cin.ignore();
            decryptFile(filename, inputBuffer, key);
            break;
        default:
            cout << "Invalid command.\n";
            break;
    }
}

void TextEditor::appendText(const char* newText) {
    textStorage->appendText(newText);
}

void TextEditor::newLine() {
    textStorage->newLine();
}

void TextEditor::saveToFile(const char* filename) {
    textStorage->saveToFile(filename);
}

void TextEditor::loadFromFile(const char* filename) {
    textStorage->loadFromFile(filename);
}

void TextEditor::printText() {
    textStorage->printText();
}

void TextEditor::insertText(int line, int index, const char* str) {
    Command* command = new InsertCommand(textStorage, line, index, str);
    command->execute();
    undoStack.push(command);
    while (!redoStack.empty()) {
        delete redoStack.top();
        redoStack.pop();
    }
}

void TextEditor::deleteText(int line, int index, int length) {
    Command* command = new DeleteCommand(textStorage, line, index, length);
    command->execute();
    undoStack.push(command);
    while (!redoStack.empty()) {
        delete redoStack.top();
        redoStack.pop();
    }
}

void TextEditor::undo() {
    if (!undoStack.empty()) {
        Command* command = undoStack.top();
        command->undo();
        undoStack.pop();
        redoStack.push(command);
    }
}

void TextEditor::redo() {
    if (!redoStack.empty()) {
        Command* command = redoStack.top();
        command->execute();
        redoStack.pop();
        undoStack.push(command);
    }
}

void TextEditor::cutText(int line, int index, int length) {
    Command* command = new CutCommand(textStorage, line, index, length, clipboard);
    command->execute();
    undoStack.push(command);
    while (!redoStack.empty()) {
        delete redoStack.top();
        redoStack.pop();
    }
}

void TextEditor::copyText(int line, int index, int length) {
    Command* command = new CopyCommand(textStorage, line, index, length, clipboard);
    command->execute();
}

void TextEditor::pasteText(int line, int index) {
    Command* command = new PasteCommand(textStorage, line, index, clipboard);
    command->execute();
    undoStack.push(command);
    while (!redoStack.empty()) {
        delete redoStack.top();
        redoStack.pop();
    }
}

void TextEditor::insertReplaceText(int line, int index, const char* str) {
    Command* command = new InsertReplaceCommand(textStorage, line, index, str);
    command->execute();
    undoStack.push(command);
    while (!redoStack.empty()) {
        delete redoStack.top();
        redoStack.pop();
    }
}

void TextEditor::searchText(const char* str) {
    textStorage->searchText(str);
}

void TextEditor::clearConsole() {
    textStorage->clearConsole();
}

void TextEditor::freeTextStorage() {
    textStorage->freeTextStorage();
}

void TextEditor::encryptFile(const char* inputFile, const char* outputFile, int key) {
    textStorage->loadFromFile(inputFile);
    FILE* file;
    errno_t err = fopen_s(&file, outputFile, "w");
    if (err != 0) {
        cerr << "Could not open file for writing.\n";
        return;
    }

    for (int i = 0; i < textStorage->getTotalLines(); i++) {
        char* text = textStorage->getText(i);
        char* encryptedText = caesarCipher->encrypt(text, key);
        fprintf(file, "%s\n", encryptedText);
    }

    fclose(file);
}

void TextEditor::decryptFile(const char* inputFile, const char* outputFile, int key) {
    textStorage->loadFromFile(inputFile);
    FILE* file;
    errno_t err = fopen_s(&file, outputFile, "w");
    if (err != 0) {
        cerr << "Could not open file for writing.\n";
        return;
    }

    for (int i = 0; i < textStorage->getTotalLines(); i++) {
        char* text = textStorage->getText(i);
        char* decryptedText = caesarCipher->decrypt(text, key);
        fprintf(file, "%s\n", decryptedText);
    }

    fclose(file);
}

int main() {
    TextEditor editor;
    editor.run();
    return 0;
}
