// Projeto Aula 06 - Henrique Sanches e Renan Alves

#include <stdio.h>
#include <string.h>

struct aluno {
    char id_aluno[4];
    char sigla_disc[4];
    char nome_aluno[50];
    char nome_disc[50];
    float media;
    float freq;
};

struct chave_primaria {
    char id_aluno[4];
    char sigla_disc[4];
    int offset;
};

struct chave_secundaria {
    char nome_aluno[50];
    int inicioLista;
};

struct lista_invertida {
    char id_aluno[4];
    char sigla_disc[4];
    int offset;
    int prox;
};
    
int arquivo_existe(char *arquivo) { // Verifica se o arquivo existe
    FILE *fd;

    if ((fd = fopen(arquivo, "rb")) == NULL) {
        return 0;
    }

    fclose(fd);

    return 1;
}

void ler_alunos_busca_s(struct aluno *chaves, int *count) { // Lê os nomes dos alunos a serem buscados
    FILE *fd;
    *count = 0;

    if ((fd = fopen("busca_s.bin", "rb")) == NULL) {
        printf("Nao foi possivel abrir o arquivo\n");
        return;
    }

    while (fread(chaves[*count].nome_aluno, sizeof(char), 50, fd) == 50) {
        printf("Nome: %s\n", chaves[*count].nome_aluno);
        (*count)++;
    }

    fclose(fd);
}

void ler_alunos_busca_p(struct aluno *chaves, int *count) { // Lê as chaves primárias dos alunos a serem buscados
    FILE *fd;
    *count = 0;

    if ((fd = fopen("busca_p.bin", "rb")) == NULL) {
        printf("Nao foi possivel abrir o arquivo\n");
        return;
    }

    while (fread(chaves[*count].id_aluno, sizeof(char), 4, fd) == 4) {
        printf("ID: %s\n", chaves[*count].id_aluno);
        fread(chaves[*count].sigla_disc, sizeof(char), 4, fd);
        printf("Sigla: %s\n", chaves[*count].sigla_disc);
        (*count)++;
    }

    fclose(fd);
}

void ler_alunos_insere(struct aluno *alunos, int *count) { // Lê os alunos a serem inseridos
    FILE *fd;
    *count = 0;

    if ((fd = fopen("insere.bin", "rb")) == NULL) {
        printf("Nao foi possivel abrir o arquivo\n");
        return;
    }

    while (fread(&alunos[*count], sizeof(struct aluno), 1, fd) == 1) {
        (*count)++;
    }

    fclose(fd);
}

int id_para_inserir() { // Guarda a posição do próximo registro a ser inserido
    FILE *fd;
    int arquivoExiste, id_inserir, id_busca_p, id_busca_s, id_inserir_novo;

    arquivoExiste = arquivo_existe("arquivoAuxiliar.bin");

    if (!arquivoExiste) {
        if ((fd = fopen("arquivoAuxiliar.bin","w+b")) == NULL) {
            printf("Nao foi possivel abrir o arquivo");
            return -1;
        }

        id_inserir = 1;
        id_busca_p = 0;
        id_busca_s = 0;
        fwrite(&id_inserir, sizeof(int), 1, fd);
        fwrite(&id_busca_p, sizeof(int), 1, fd);
        fwrite(&id_busca_s, sizeof(int), 1, fd);
        fclose(fd);
        return 0;
    }

    if ((fd = fopen("arquivoAuxiliar.bin","r+b")) == NULL) {
        printf("Nao foi possivel abrir o arquivo");
        return -1;
    }

    fread(&id_inserir, sizeof(int), 1, fd);
    id_inserir_novo = id_inserir + 1;

    fseek(fd, 0, SEEK_SET);
    fwrite(&id_inserir_novo, sizeof(int), 1, fd);
    fclose(fd);
    return id_inserir;
}

int id_para_busca_p() { // Guarda a posição do próximo registro a ser buscado por chave primária
    FILE *fd;
    int id, id_novo;

    if ((fd = fopen("arquivoAuxiliar.bin","r+b")) == NULL) {
        printf("Nao foi possivel abrir o arquivo");
        return -1;
    }

    fseek(fd, sizeof(int), SEEK_SET);
    fread(&id, sizeof(int), 1, fd);
    id_novo = id + 1;

    fseek(fd, sizeof(int), SEEK_SET);
    fwrite(&id_novo, sizeof(int), 1, fd);
    fclose(fd);
    return id;
}

int id_para_busca_s() { // Guarda a posição do próximo registro a ser buscado por chave secundária
    FILE *fd;
    int id, id_novo;

    if ((fd = fopen("arquivoAuxiliar.bin","r+b")) == NULL) {
        printf("Nao foi possivel abrir o arquivo");
        return -1;
    }

    fseek(fd, 2 * (sizeof(int)), SEEK_SET);
    fread(&id, sizeof(int), 1, fd);
    id_novo = id + 1;

    fseek(fd, 2 * (sizeof(int)), SEEK_SET);
    fwrite(&id_novo, sizeof(int), 1, fd);
    fclose(fd);
    return id;
}

void escrever_aluno(struct aluno aluno, int tamRegistro, FILE *fd) { // Escreve um registro no arquivo
    fwrite(&tamRegistro, sizeof(int), 1, fd);
    fwrite(aluno.id_aluno, sizeof(char), 4, fd);
    fwrite("#", sizeof(char), 1, fd);
    fwrite(aluno.sigla_disc, sizeof(char), 4, fd);
    fwrite("#", sizeof(char), 1, fd);
    fwrite(aluno.nome_aluno, sizeof(char), strlen(aluno.nome_aluno) + 1, fd);
    fwrite("#", sizeof(char), 1, fd);
    fwrite(aluno.nome_disc, sizeof(char), strlen(aluno.nome_disc) + 1, fd);
    fwrite("#", sizeof(char), 1, fd);
    fwrite(&aluno.media, sizeof(float), 1, fd);
    fwrite("#", sizeof(char), 1, fd);
    fwrite(&aluno.freq, sizeof(float), 1, fd);
}

void escrever_indice(struct chave_primaria chave_p, FILE *fd) { // Escreve chave primária no arquivo índice
    fwrite(chave_p.id_aluno, sizeof(char), 4, fd);
    fwrite(chave_p.sigla_disc, sizeof(char), 4, fd);
    fwrite(&chave_p.offset, sizeof(int), 1, fd);
}

void ler_campo_var(FILE *fd, char campo[]) { // Lê campos de tamanho variável no arquivo de dados
    int i = 0;
    char ch;

    fread(&ch, sizeof(char), 1, fd);

    while(ch != '#') {
        campo[i] = ch;
        i++;
        
        fread(&ch, sizeof(char), 1, fd);
    }
}

int verificar_nome(const char *nome, struct chave_secundaria *chaves_s, int count_s) { // Verifica se o nome já foi inserido
    for (int i = 0; i < count_s; i++) {
        if (strcmp(chaves_s[i].nome_aluno, nome) == 0) {
            return i; // Nome encontrado
        }
    }
    return -1; // Nome não encontrado
}

int comparar_chaves(const void *a, const void *b) {
    struct chave_primaria *chaveA = (struct chave_primaria *)a;
    struct chave_primaria *chaveB = (struct chave_primaria *)b;

    int cmp = strncmp(chaveA->id_aluno, chaveB->id_aluno, 4);
    if (cmp == 0) {
        return strncmp(chaveA->sigla_disc, chaveB->sigla_disc, 4);
    }
    return cmp;
}

int comparar_chaves_secundarias(const void *a, const void *b) {
    struct chave_secundaria *chaveA = (struct chave_secundaria *)a;
    struct chave_secundaria *chaveB = (struct chave_secundaria *)b;
    return strcmp(chaveA->nome_aluno, chaveB->nome_aluno);
}

void ordenar_chaves_primarias(struct chave_primaria *chaves_p, int count) {
    qsort(chaves_p, count, sizeof(struct chave_primaria), comparar_chaves);
}

void ordenar_chaves_secundarias(struct chave_secundaria *chaves_s, int count) {
    qsort(chaves_s, count, sizeof(struct chave_secundaria), comparar_chaves_secundarias);
}

void inserir_chave_secundaria(struct aluno aluno, struct chave_secundaria *chaves_s, int *count_s1, struct lista_invertida *lista_s, int *count_s2, int offset) { //Insere chave secundária em vetores na memória
    int posicaoNome;

    printf("Contador 1: %d\n", *count_s1);
    printf("Contador 2: %d\n", *count_s2);

    posicaoNome = verificar_nome(aluno.nome_aluno, chaves_s, *count_s1);
    printf("PosicaoNome: %d\n", posicaoNome);

    strcpy(lista_s[*count_s2].id_aluno, aluno.id_aluno);
    strcpy(lista_s[*count_s2].sigla_disc, aluno.sigla_disc);
    lista_s[*count_s2].offset = offset;

    if (posicaoNome != -1) {
        lista_s[*count_s2].prox = chaves_s[posicaoNome].inicioLista;
        printf("Prox: %d\n", lista_s[*count_s2].prox);
        chaves_s[posicaoNome].inicioLista = *count_s2;
        printf("InicioLista: %d\n", chaves_s[posicaoNome].inicioLista);
    }
    else {
        lista_s[*count_s2].prox = -1;
        strcpy(chaves_s[*count_s1].nome_aluno, aluno.nome_aluno);
        chaves_s[*count_s1].inicioLista = *count_s2;
        printf("InicioLista: %d\n", chaves_s[*count_s1].inicioLista);
        (*count_s1)++;
    }

    (*count_s2)++;
}

void inserir_chave_primaria(struct aluno aluno, struct chave_primaria *chaves_p, int *count_p, int offset) { //Insere chave primária em um vetor na memória
    strcpy(chaves_p[*count_p].id_aluno, aluno.id_aluno);
    strcpy(chaves_p[*count_p].sigla_disc, aluno.sigla_disc);
    chaves_p[*count_p].offset = offset;
    (*count_p)++;
}

int inserir_aluno(struct aluno aluno) { // Insere um aluno no final do arquivo de dados
    FILE *fd;
    int arquivoExiste, tamRegistro, offset;

    tamRegistro = 4 + 4 + strlen(aluno.nome_aluno) + 1 + strlen(aluno.nome_disc) + 1 + sizeof(float) + sizeof(float) + 5;

    if ((fd = fopen("lista_historicos.bin","r+b")) == NULL) {
        printf("Nao foi possivel abrir o arquivo");
        return -1;
    }

    fseek(fd, 0, SEEK_END);
    escrever_aluno(aluno, tamRegistro, fd); // Grava o resgistro no final do arquivo
    offset = ftell(fd) - tamRegistro;
    fclose(fd);

    return offset;
}

int verifica_flag() { //Verifica se o programa foi interrompido na execução anterior
    FILE *fd;
    int flag, arquivoExiste, nova_flag = 1;

    arquivoExiste = arquivo_existe("lista_historicos.bin");

    if (!arquivoExiste) {
        if ((fd = fopen("lista_historicos.bin", "wb")) == NULL) {
            printf("Nao foi possivel abrir o arquivo\n");
            return 0;
        }

        flag = 0;
    }
    else {
        if ((fd = fopen("lista_historicos.bin", "r+b")) == NULL) {
            printf("Nao foi possivel abrir o arquivo\n");
            return 0;
        }

        fseek(fd, 0, SEEK_SET);
        fread(&flag, sizeof(int), 1, fd);
    }

    fseek(fd, 0, SEEK_SET);
    fwrite(&nova_flag, sizeof(int), 1, fd);
    fclose(fd);

    return flag;
}

void atualiza_flag() { //Grava no header do arquivo de dados que o programa não foi interrompido
    FILE *fd;
    int flag = 0;

    if ((fd = fopen("lista_historicos.bin", "r+b")) == NULL) {
        printf("Nao foi possivel abrir o arquivo\n");
        return;
    }

    fseek(fd, 0, SEEK_SET);
    fwrite(&flag, sizeof(int), 1, fd);

    fclose(fd);
}

void ler_indice_s(struct chave_secundaria *chaves_s, int *count1, struct lista_invertida *lista_s, int *count2) { //Carrega as chaves secundárias para vetores na memória
    FILE *fd;

    if ((fd = fopen("lista_indices_secundarios.bin", "rb")) == NULL) {
        printf("Nao foi possivel abrir o arquivo\n");
        return;
    }

    while (fread(chaves_s[*count1].nome_aluno, sizeof(char), 50, fd) == 50) {
        fread(&chaves_s[*count1].inicioLista, sizeof(int), 1, fd);
        (*count1)++;
    }

    fclose(fd);

    if ((fd = fopen("lista_invertida.bin", "rb")) == NULL) {
        printf("Nao foi possivel abrir o arquivo\n");
        return;
    }

    while (fread(&lista_s[*count2], sizeof(struct lista_invertida), 1, fd) == 1) {
        (*count2)++;
    }

    fclose(fd);
}

void ler_indice(struct chave_primaria *chaves_p, int *count) { //Carrega as chaves primárias para um vetor na memória
    FILE *fd;

    if ((fd = fopen("lista_indices.bin", "rb")) == NULL) {
        printf("Nao foi possivel abrir o arquivo\n");
        return;
    }

    while (fread(&chaves_p[*count], sizeof(struct chave_primaria), 1, fd) == 1) {
        (*count)++;
    }

    fclose(fd);
}

void carrega_indice_s(struct chave_secundaria *chaves_s, int count1, struct lista_invertida *lista_s, int count2) { // Carrega as chaves secundárias para os arquivos de índice
    FILE *fd;

    ordenar_chaves_secundarias(chaves_s, count1);

    if ((fd = fopen("lista_indices_secundarios.bin", "wb")) == NULL) {
        printf("Nao foi possivel abrir o arquivo\n");
        return;
    }

    for (int i = 0; i < count1; i++) {
        fwrite(chaves_s[i].nome_aluno, sizeof(char), 50, fd);
        fwrite(&chaves_s[i].inicioLista, sizeof(int), 1, fd);
    }

    fclose(fd);

    if ((fd = fopen("lista_invertida.bin", "wb")) == NULL) {
        printf("Nao foi possivel abrir o arquivo\n");
        return;
    }

    for (int i = 0; i < count2; i++) {
        fwrite(&lista_s[i], sizeof(struct lista_invertida), 1, fd);
    }

    fclose(fd);
}

void carrega_indice(struct chave_primaria *chaves_p, int count) { // Carrega as chaves primárias para o arquivo de índice
    FILE *fd;

    ordenar_chaves_primarias(chaves_p, count);

    if ((fd = fopen("lista_indices.bin", "wb")) == NULL) {
        printf("Nao foi possivel abrir o arquivo\n");
        return;
    }

    for (int i = 0; i < count; i++) {
        escrever_indice(chaves_p[i], fd);
    }

    fclose(fd);
}

void recria_indice_s(struct chave_secundaria *chaves_s, int *count1, struct lista_invertida *lista_s, int *count2) { // Lê o arquivo de dados e recria as chaves secundárias já que os arquivos de índice estão desatualizados
    FILE *fd;
    struct aluno aluno;
    char campo_var[50];
    int tamRegistro, offset;

    if ((fd = fopen("lista_historicos.bin", "r+b")) == NULL) {
        printf("Nao foi possivel abrir o arquivo\n");
        return;
    }

    fseek(fd, 4, SEEK_SET);

    while (fread(&tamRegistro, sizeof(int), 1, fd) == 1) {
        offset = ftell(fd);

        fread(&aluno.id_aluno, sizeof(char), 4, fd);
        fseek(fd, sizeof(char), SEEK_CUR);

        fread(&aluno.sigla_disc, sizeof(char), 4, fd);
        fseek(fd, sizeof(char), SEEK_CUR);

        ler_campo_var(fd, campo_var);
        strcpy(aluno.nome_aluno, campo_var);

        ler_campo_var(fd, campo_var);
        strcpy(aluno.nome_disc, campo_var);

        fread(&aluno.media, sizeof(float), 1, fd);
        fseek(fd, sizeof(char), SEEK_CUR);

        fread(&aluno.freq, sizeof(float), 1, fd);

        inserir_chave_secundaria(aluno, chaves_s, count1, lista_s, count2, offset);
    }

    fclose(fd);
}

void recria_indice(struct chave_primaria *chaves_p, int *count) { // Lê o arquivo de dados e recria as chaves primárias já que o arquivo de índice está desatualizado
    FILE *fd;
    char id_aluno[4], sigla_disc[4];
    int tamRegistro, offset;
    
    if ((fd = fopen("lista_historicos.bin", "r+b")) == NULL) {
        printf("Nao foi possivel abrir o arquivo\n");
        return;
    }

    fseek(fd, 4, SEEK_SET);

    while (fread(&tamRegistro, sizeof(int), 1, fd) == 1) { // Percorre o arquivo
        offset = ftell(fd);
        fread(&id_aluno, sizeof(id_aluno), 1, fd);
        fseek(fd, sizeof(char), SEEK_CUR);
        fread(&sigla_disc, sizeof(sigla_disc), 1, fd);

        strcpy(chaves_p[*count].id_aluno, id_aluno);
        strcpy(chaves_p[*count].sigla_disc, sigla_disc);
        chaves_p[*count].offset = offset;

        (*count)++;

        fseek(fd, tamRegistro - sizeof(id_aluno) - sizeof(sigla_disc) - sizeof(char), SEEK_CUR);
    }

    fclose(fd);
    ordenar_chaves_primarias(chaves_p, *count);
}

void busca_secundaria(char *nome, struct chave_secundaria *chaves_s, int count1, struct lista_invertida *lista_s) { // Busca um aluno pelo nome
    FILE *fd;
    struct aluno pesquisa;
    char campo_var[50];
    int prox, j = 1;
    
    for (int i = 0; i < count1; i++) {
        if (strcmp(chaves_s[i].nome_aluno, nome) == 0) {
            if((fd = fopen("lista_historicos.bin", "rb")) == NULL) {
                printf("Nao foi possivel abrir o arquivo\n");
                return;
            }

            prox = chaves_s[i].inicioLista;

            while (prox != -1) {
                fseek(fd, lista_s[prox].offset, SEEK_SET);

                printf("\nAluno %d:\n", j);

                fread(&pesquisa.id_aluno, sizeof(char), 4, fd);
                printf("ID: %s\n", pesquisa.id_aluno);
                fseek(fd, sizeof(char), SEEK_CUR);

                fread(&pesquisa.sigla_disc, sizeof(char), 4, fd);
                printf("Sigla: %s\n", pesquisa.sigla_disc);
                fseek(fd, sizeof(char), SEEK_CUR);

                ler_campo_var(fd, campo_var);
                strcpy(pesquisa.nome_aluno, campo_var);
                printf("Nome: %s\n", pesquisa.nome_aluno);

                ler_campo_var(fd, campo_var);
                strcpy(pesquisa.nome_disc, campo_var);
                printf("Disciplina: %s\n", pesquisa.nome_disc);

                fread(&pesquisa.media, sizeof(float), 1, fd);
                printf("Media: %.2f\n", pesquisa.media);
                fseek(fd, sizeof(char), SEEK_CUR);

                fread(&pesquisa.freq, sizeof(float), 1, fd);
                printf("Frequencia: %.2f\n", pesquisa.freq);

                prox = lista_s[prox].prox;
                j++;
            }

            fclose(fd);

            return;
        }
    }

    printf("Aluno(s) não encontrado(s)\n");
}

void busca_primaria(struct aluno *pesquisa, struct chave_primaria *chaves_p, int count) { // Busca um aluno pela chave primária
    FILE *fd;
    char campo_var[50];

    for (int i = 0; i < count; i++) {
        if (strncmp(chaves_p[i].id_aluno, pesquisa->id_aluno, 4) == 0 && strncmp(chaves_p[i].sigla_disc, pesquisa->sigla_disc, 4) == 0) {
            if((fd = fopen("lista_historicos.bin", "rb")) == NULL) {
                printf("Nao foi possivel abrir o arquivo\n");
                return;
            }

            fseek(fd, chaves_p[i].offset, SEEK_SET);

            fread(&pesquisa->id_aluno, sizeof(char), 4, fd);
            printf("ID: %s\n", pesquisa->id_aluno);
            fseek(fd, sizeof(char), SEEK_CUR);

            fread(&pesquisa->sigla_disc, sizeof(char), 4, fd);
            printf("Sigla: %s\n", pesquisa->sigla_disc);
            fseek(fd, sizeof(char), SEEK_CUR);

            ler_campo_var(fd, campo_var);
            strcpy(pesquisa->nome_aluno, campo_var);
            printf("Nome: %s\n", pesquisa->nome_aluno);

            ler_campo_var(fd, campo_var);
            strcpy(pesquisa->nome_disc, campo_var);
            printf("Disciplina: %s\n", pesquisa->nome_disc);

            fread(&pesquisa->media, sizeof(float), 1, fd);
            printf("Media: %.2f\n", pesquisa->media);
            fseek(fd, sizeof(char), SEEK_CUR);

            fread(&pesquisa->freq, sizeof(float), 1, fd);
            printf("Frequencia: %.2f\n", pesquisa->freq);

            fclose(fd);

            return;
        }
    }

    printf("Aluno não encontrado\n");
}

int main() {
    struct aluno alunos[100], chaves_p_busca[100], chaves_s_busca[100];
    struct chave_primaria chaves_p_atuais[100];
    struct chave_secundaria chaves_s_atuais[100];
    struct lista_invertida lista_s[100];
    int count_insere, id, i = 0, j = 0;
    int count_p_busca, count_p_atuais = 0;
    int count_s_busca, count_s_atuais = 0, count_lista = 0;
    int flag, offset;
    char entrada;

    flag = verifica_flag();

    if (flag) {
        recria_indice(chaves_p_atuais, &count_p_atuais);
        recria_indice_s(chaves_s_atuais, &count_s_atuais, lista_s, &count_lista);
    }
    else {
        ler_indice(chaves_p_atuais, &count_p_atuais);
        ler_indice_s(chaves_s_atuais, &count_s_atuais, lista_s, &count_lista);
    }

    ler_alunos_insere(alunos, &count_insere);
    ler_alunos_busca_p(chaves_p_busca, &count_p_busca);
    ler_alunos_busca_s(chaves_s_busca, &count_s_busca);

    /* for (i = 0; i < count_insere; i++) {
        inserir_aluno(alunos[i], chaves_p, &count_chaves_p);
    }

    carrega_indice(chaves_p, &count_chaves_p);
    recria_indice(chaves_p, &count_chaves_p); */

    entrada = '0';

    while (entrada != 'e') {
        printf("\nOpções: \n\n");
        printf("1 - Inserir aluno(i)\n");
        printf("2 - Busca por chave primária(p)\n");
        printf("3 - Busca por chave secundária(s)\n");
        printf("4 - Salvar alterações(e)\n");
        printf("\nDigite a opção desejada: ");

        entrada = getchar();
        getchar();

        switch (entrada) {
            case 'i':
                if (i < (count_insere - 1)) {
                    id = id_para_inserir();
                    offset = inserir_aluno(alunos[id]);
                    inserir_chave_primaria(alunos[id], chaves_p_atuais, &count_p_atuais, offset);
                    inserir_chave_secundaria(alunos[id], chaves_s_atuais, &count_s_atuais, lista_s, &count_lista, offset);
                    i = id;
                }
                else {
                    printf("Todos os alunos já foram inseridos\n");
                }
                break;
            case 'p':
                if (j < (count_p_busca - 1)) {
                    id = id_para_busca_p();
                    busca_primaria(&chaves_p_busca[id], chaves_p_atuais, count_p_atuais);
                    j = id;
                }
                else {
                    printf("Todos os alunos já foram buscados\n");
                }
                break;
            case 's':
                if (j < (count_s_busca - 1)) {
                    id = id_para_busca_s();
                    busca_secundaria(chaves_s_busca[id].nome_aluno, chaves_s_atuais, count_s_atuais, lista_s);
                    j = id;
                }
                else {
                    printf("Todos os alunos já foram buscados\n");
                }
                break;
            case 'e':
                entrada = 'e';
                carrega_indice(chaves_p_atuais, count_p_atuais);
                carrega_indice_s(chaves_s_atuais, count_s_atuais, lista_s, count_lista);
                atualiza_flag();
                break;
            default:
                printf("Opção inválida\n");
                break;
        }
    }

    return 0;
}
