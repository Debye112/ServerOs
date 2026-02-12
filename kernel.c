
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

//uart_puts() = print.
//uart_gets() = input.
//strlen() = returns string lenght.
//stringcom() = if strings are equal returns 1 if not returns -1.

// ======================= UART & Types =======================
#define UART0_BASE 0x100201000UL

#define UART_DR (*(volatile uint32_t *)(UART0_BASE + 0x00)) 
#define UART_FR (*(volatile uint32_t *)(UART0_BASE + 0x18))

#define UART_FR_TXFF (1 << 5)   
#define UART_FR_RXFE (1 << 4)   

#define MAX_INPUT 128

#define BLOCK_SIZE 512

int currentDirectory;
int poolIndex = 0;

struct nodes{
    char nodeName[32];
    int size;
    int layer;
    int parentIndex;
};

#define NODE_BLOCK 1   // disk block where nodes live

struct nodes nodePool[128];

struct commandInputs{
    char command[32];
    int commandSize;
    char name[32];
    int nameSize;
};

int poolNumber = 0;

// --- temporary stubs so it builds without disk I/O ---
void loadNodes() { }
void saveNodes() { }
// ------------------------------------------------------

void uart_putc(char c) {
    while (UART_FR & UART_FR_TXFF);  
    UART_DR = c;
}

void uart_puts(const char* s) {
    while (*s) {
        uart_putc(*s++);
    }
}

char uart_getc() {
    while (UART_FR & UART_FR_RXFE);  
    return (char)(UART_DR & 0xFF);
}

void uart_gets(char* buf) {
    int i = 0;

    while (i < MAX_INPUT - 1) {
        char c = uart_getc();
        if (c == '\r' || c == '\n') {
            uart_putc('\n');
            break;
        }
        if (c == '\b' || c == 127) {
            if (i > 0) {
                i--;
                uart_puts("\b \b");
            }
            continue;
        }

        buf[i++] = c;
        uart_putc(c);
    }

    buf[i] = '\0';
}

int strlen(char *resp){
    int index = 0;
    while (resp[index] != '\0'){
        index++;
    }
    return index;   
}

int nameBoundCheck(char *name){
    if (strlen(name) < 32) return 1;
    return 0;
}

int stringcom(char *comref, int s_comref, char *com, int s_com){
    int relevance = 0;
    if(s_comref == s_com){

        for(int i = 0; i < s_com; i++){
            if(comref[i] == com[i]) relevance++;
        }

        if(relevance == s_com) return 1;
        
        else 
            return -1;
    }
    else 
        return -1;
    
}

void newDirectory(char *name){
    if(++poolIndex < 128){ 
            struct nodes *n = &nodePool[poolIndex];
        
            int i = 0;
            while (name[i]) {
                n->nodeName[i] = name[i];
                i++;
            }
            n->nodeName[i] = '\0';
        
            n->size = 0;
            n->layer = nodePool[currentDirectory].layer + 1;
            n->parentIndex = currentDirectory;
    }
    else
        uart_puts("Internal Nodes ran out. Contact the os owner");
}

void list(){

    for(int i =0; i <= poolIndex; i++){
        if(nodePool[i].parentIndex == currentDirectory){
            uart_puts(nodePool[i].nodeName);
            uart_puts("  ");
        }
    }
    
}

void commandDisecter(struct commandInputs *c, char *input) {
    int i = 1, p = 0;

    while (input[i] && input[i] != '/') {
        c->command[p++] = input[i++];
    }
    c->command[p] = '\0';
    c->commandSize = p;

    if (input[i] == '/') {
        i++;
        p = 0;
        while (input[i]) {
            c->name[p++] = input[i++];
        }
        c->name[p] = '\0';
        c->nameSize = p;
    }
}

void changeDirect(char *name) {

    if(stringcom(name, strlen(name), "..", 2) == 1){
        currentDirectory = nodePool[currentDirectory].parentIndex;
    }
    else if(stringcom(name, strlen(name), "./", 2) == 1){
            currentDirectory = 0;
        }
    else{   
        for (int i = 0; i <= poolIndex; i++) {
            if (nodePool[i].parentIndex == currentDirectory) {
                if (stringcom(name, strlen(name), nodePool[i].nodeName, strlen(nodePool[i].nodeName)) == 1) {
                    currentDirectory = i;
                    return;
                }
            }
        }
        uart_puts("cd: directory not found\n");
    }
}

void removeDirectory(char *name){
    int found = 0;

    for (int i = 0; i <= poolIndex; i++) {
        if (nodePool[i].parentIndex == currentDirectory && stringcom(name, strlen(name), nodePool[i].nodeName, strlen(nodePool[i].nodeName)) == 1) {
            
            found = 1;
            for (int j = i; j < poolIndex; j++) {
                nodePool[j] = nodePool[j + 1];
            }
            poolIndex--; 
            break;       
        }
    }

    if (!found) {
        uart_puts("rm: directory not found\n");
    }
}

int copyDirectory(char *name){

    for(int i = 0; i <=poolIndex; i++){
        if (nodePool[i].parentIndex == currentDirectory && stringcom(name, strlen(name), nodePool[i].nodeName, strlen(nodePool[i].nodeName)) == 1)
            return i;
    }
    return -1;
}

void pasteDirectory(int poolNumber){
    nodePool[poolNumber].parentIndex = currentDirectory;
}

int commands(char *input) {
    int inputSize = strlen(input);
    if (input[0] != '/') {
        char ls[] = "ls";
        if (stringcom(ls, 2, input, inputSize) == 1) {
            list();
            return 1;
        }
        char exit[] = "exit";
        if (stringcom(exit, 4, input, inputSize) == 1) {
            return -1;
        }

        uart_puts("Unknown command\n");
        return 1;
    }

    struct commandInputs comInput;
    commandDisecter(&comInput, input);
    
    char mkdir[] = "mkdir";
    if (stringcom(mkdir, 5, comInput.command, comInput.commandSize) == 1) {
        if(nameBoundCheck(comInput.name) == 1){
            newDirectory(comInput.name);
        }
        else{
                uart_puts("Name exceeds limit");
        }
    }
    char cd[] = "cd";
    if (stringcom(cd, 2, comInput.command, comInput.commandSize) == 1) {
        changeDirect(comInput.name);
        saveNodes();
        return 1;
    }
    char rm[] = "rm";
        if (stringcom(rm, 2, comInput.command, comInput.commandSize) == 1) {
            removeDirectory(comInput.name);
            saveNodes();
            return 1;
    }
    char copy[] = "copy";
        if (stringcom(copy, 4, comInput.command, comInput.commandSize) == 1) {
            poolNumber = copyDirectory(comInput.name);
            if(poolNumber < 0) uart_puts("Copying failed");
            saveNodes();
            return 1;
    }
    char paste[] = "paste";
        if (stringcom(paste, 5, comInput.command, comInput.commandSize) == 1) {
            pasteDirectory(poolNumber);
            saveNodes();
            return 1;
        }

    return 1;
}


// -------- Kernel entry --------
void kernel_main() {

    int security_code = 0;
    char input_pass[64];
    char pass[] = "jacknotcool";
    
    while(security_code == 0) {
        uart_puts("\nEnter OS password:");
        uart_gets(input_pass);
        int correction = stringcom(pass, strlen(pass), input_pass, strlen(input_pass));
        if(correction == 1) break;
    }
    char buffer[MAX_INPUT];
    loadNodes();
    int jk = 0;
    char name[] = "root";
    
    while (name[jk]) {
        nodePool[0].nodeName[jk] = name[jk];
        jk++;
    }
    nodePool[0].nodeName[jk] = '\0';
    nodePool[0].size = 0;
    nodePool[0].layer = 1;
    nodePool[0].parentIndex = -1;
    
    currentDirectory = 0;
    int loop_break = 0;
    while(loop_break != -1){

            uart_puts("\n");
            uart_puts("leos/");
            uart_puts(nodePool[currentDirectory].nodeName);
            uart_puts("> ");
            
        
            uart_gets(buffer);

            loop_break = commands(buffer);
    }

    while(1) { }
}

