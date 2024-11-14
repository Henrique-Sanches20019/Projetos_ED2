// Projeto Aula 10 - Henrique Sanches e Renan Alves

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define HASH_TABLE_SIZE 13
#define HASH_SLOT_SIZE 12

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
};

int arquivo_existe(char *arquivo) { // Verifica se o arquivo existe
    FILE *fd;

    if ((fd = fopen(arquivo, "rb")) == NULL) {
        return 0;
    }

    fclose(fd);

    return 1;
}

void ler_alunos(struct aluno *alunos, int *count) { // Lê um arquivo de alunos
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

void ler_chaves_primarias(struct chave_primaria *alunos, int *count, char arquivo) { // Lê um arquivo de chaves primárias
    FILE *fd;
    *count = 0;

    if (arquivo == 'r') {
        if ((fd = fopen("remove.bin", "rb")) == NULL) {
            printf("Nao foi possivel abrir o arquivo\n");
            return;
        }
    }
    else {
        if ((fd = fopen("busca.bin", "rb")) == NULL) {
            printf("Nao foi possivel abrir o arquivo\n");
            return;
        }
    }

    while (fread(&alunos[*count], sizeof(struct chave_primaria), 1, fd) == 1) {
        (*count)++;
    }

    fclose(fd);
}

int id_para_inserir() { // Guarda a posição do próximo registro a ser inserido
    FILE *fd;
    int arquivoExiste, id_inserir, id_remover, id_buscar, id_inserir_novo;

    arquivoExiste = arquivo_existe("arquivoAuxiliar.bin");

    if (!arquivoExiste) {
        if ((fd = fopen("arquivoAuxiliar.bin","w+b")) == NULL) {
            printf("Nao foi possivel abrir o arquivo");
            return -1;
        }

        id_inserir = 1;
        id_remover = 0;
        id_buscar = 0;
        fwrite(&id_inserir, sizeof(int), 1, fd);
        fwrite(&id_remover, sizeof(int), 1, fd);
        fwrite(&id_buscar, sizeof(int), 1, fd);
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

int id_para_remover() { // Guarda a posição do próximo registro a ser removido
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

int id_para_buscar() { // Guarda a posição do próximo registro a ser buscado
    FILE *fd;
    int id, id_novo;

    if ((fd = fopen("arquivoAuxiliar.bin","r+b")) == NULL) {
        printf("Nao foi possivel abrir o arquivo");
        return -1;
    }

    fseek(fd, 2 * sizeof(int), SEEK_SET);
    fread(&id, sizeof(int), 1, fd);
    id_novo = id + 1;

    fseek(fd, 2 * sizeof(int), SEEK_SET);
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

void escrever_chave_primaria(char *id_aluno, char *sigla_disc, int offset, FILE *fd) { // Escreve uma chave primária no arquivo da tabela hash
    fwrite(id_aluno, sizeof(char), 4, fd);
    fwrite(sigla_disc, sizeof(char), 4, fd);
    fwrite(&offset, sizeof(int), 1, fd);
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

int funcao_hash(char *id_aluno, char *sigla_disc) { // Função de hash
    int i, hash = 0;

    for (i = 0; i < 4; i++) {
        hash += id_aluno[i];
    }

    for (i = 0; i < 4; i++) {
        hash += sigla_disc[i];
    }

    return hash % HASH_TABLE_SIZE;
}

void inserir_hash(char *id_aluno, char *sigla_disc, int offset) {
    FILE *fd;
    int arquivoExiste, hash, newHash, tentativas = 0;
    int flagSpaceFound = 0;
    char espacoLivre = '/', leitura;

    arquivoExiste = arquivo_existe("hash_table.bin");

    if (!arquivoExiste) {
        if ((fd = fopen("hash_table.bin","w+b")) == NULL) {
            printf("Nao foi possivel abrir o arquivo");
            return;
        }

        for (int i = 0; i < HASH_TABLE_SIZE * 2; i++) {
            fwrite(&espacoLivre, HASH_SLOT_SIZE, 1, fd);
        }
    }
    else {
        if ((fd = fopen("hash_table.bin","r+b")) == NULL) {
            printf("Nao foi possivel abrir o arquivo");
            return;
        }
    }

    hash = funcao_hash(id_aluno, sigla_disc);
    newHash = hash;

    fseek(fd, hash * (HASH_SLOT_SIZE * 2), SEEK_SET);

    while (!flagSpaceFound) {
        printf("Endereço %d\n", newHash);

        for (int i = 0; i < 2; i++) {
            fread(&leitura, sizeof(leitura), 1, fd);
            fseek(fd, -sizeof(leitura), SEEK_CUR);

            if (leitura == '/' || leitura == '#') {
                escrever_chave_primaria(id_aluno, sigla_disc, offset, fd);
                printf("Chave inserida com sucesso\n");
                fclose(fd);
                return;
            }
            else {
                if (i == 0 && newHash == hash) 
                    printf("Colisão\n");

                fseek(fd, HASH_SLOT_SIZE, SEEK_CUR);
            }

            tentativas++;
            printf("Tentativa %d\n", tentativas);
        }

        newHash++;

        if (newHash == HASH_TABLE_SIZE) {
            newHash = 0;
            fseek(fd, 0, SEEK_SET);
        }

        if (newHash == hash) {
            printf("Tabela hash cheia\n");
            flagSpaceFound = 1;
        }
    }

    fclose(fd);
}

void inserir_aluno(struct aluno aluno) {
    FILE *fd;
    int tamRegistro;

    tamRegistro = 4 + 4 + strlen(aluno.nome_aluno) + 1 + strlen(aluno.nome_disc) + 1 + sizeof(float) + sizeof(float) + 5;

    if ((fd = fopen("lista_historicos.bin","ab")) == NULL) {
        printf("Nao foi possivel abrir o arquivo");
        return;
    }

    inserir_hash(aluno.id_aluno, aluno.sigla_disc, ftell(fd));
    escrever_aluno(aluno, tamRegistro, fd);
    fclose(fd);
}

void remover_hash(struct chave_primaria aluno) {
    FILE *fd;
    int hash, newHash, offset;
    int flagSpaceFound = 0;
    char leitura, espacoRemovido = '#', id_aluno[4], sigla_disc[4];

    if ((fd = fopen("hash_table.bin","r+b")) == NULL) {
        printf("Nao foi possivel abrir o arquivo");
        return;
    }

    hash = funcao_hash(aluno.id_aluno, aluno.sigla_disc);
    newHash = hash;

    fseek(fd, hash * (HASH_SLOT_SIZE * 2), SEEK_SET);

    while (!flagSpaceFound) {
        for (int i = 0; i < 2; i++) {
            fread(&id_aluno, sizeof(id_aluno), 1, fd);
            fread(&sigla_disc, sizeof(sigla_disc), 1, fd);
            fread(&offset, sizeof(int), 1, fd);

            if (id_aluno[0] == '/') {
                printf("Registro não encontrado\n");
                fclose(fd);
                return;
            }
            else if (strncmp(id_aluno, aluno.id_aluno, 4) == 0 && strncmp(sigla_disc, aluno.sigla_disc, 4) == 0) {
                fseek(fd, -HASH_SLOT_SIZE, SEEK_CUR);
                fwrite(&espacoRemovido, HASH_SLOT_SIZE, 1, fd);

                fclose(fd);
                return;
            }
        }

        newHash++;

        if (newHash == HASH_TABLE_SIZE) {
            newHash = 0;
            fseek(fd, 0, SEEK_SET);
        }

        if (newHash == hash) {
            printf("Registro não encontrado\n");
            flagSpaceFound = 1;
        }
    }

    fclose(fd);
}

int buscar_hash(struct chave_primaria aluno) {
    FILE *fd;
    int hash, newHash, offset, acessos = 0;
    int flagSpaceFound = 0;
    char leitura, espacoRemovido = '#', id_aluno[4], sigla_disc[4];

    if ((fd = fopen("hash_table.bin","rb")) == NULL) {
        printf("Nao foi possivel abrir o arquivo");
        return -1;
    }

    hash = funcao_hash(aluno.id_aluno, aluno.sigla_disc);
    newHash = hash;

    fseek(fd, hash * (HASH_SLOT_SIZE * 2), SEEK_SET);

    while (!flagSpaceFound) {
        for (int i = 0; i < 2; i++) {
            fread(&id_aluno, sizeof(id_aluno), 1, fd);
            fread(&sigla_disc, sizeof(sigla_disc), 1, fd);
            fread(&offset, sizeof(int), 1, fd);
            acessos++;

            if (id_aluno[0] == '/') {
                fclose(fd);
                return -1;
            }  
            else if (strncmp(id_aluno, aluno.id_aluno, 4) == 0 && strncmp(sigla_disc, aluno.sigla_disc, 4) == 0) {
                fread(&offset, sizeof(int), 1, fd);

                printf("Registro encontrado, enderço %d, %d acessos\n", newHash, acessos);

                fclose(fd);
                return offset;
            }
        }

        newHash++;

        if (newHash == HASH_TABLE_SIZE) {
            newHash = 0;
            fseek(fd, 0, SEEK_SET);
        }

        if (newHash == hash) {
            printf("Registro não encontrado\n");
            flagSpaceFound = 1;
        }
    }

    fclose(fd);
}

void buscar_aluno(struct chave_primaria aluno) {
    FILE *fd;
    int offset, tamRegistro;
    char campo_var[50];
    struct aluno pesquisa;
    
    offset = buscar_hash(aluno);

    if (offset == -1) {
        printf("Registro não encontrado\n");
        return;
    }

    if ((fd = fopen("lista_historicos.bin","rb")) == NULL) {
        printf("Nao foi possivel abrir o arquivo");
        return;
    }

    /* fseek(fd, offset, SEEK_SET);
    fread(&tamRegistro, sizeof(int), 1, fd);

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
    
    mudei dnv */

    fclose(fd);
    
}

int main() {
    struct aluno alunos[100];
    struct chave_primaria chaves_remove[100], chaves_busca[100];
    int count_insere, count_remove, count_busca, id;
    char entrada;

    ler_alunos(alunos, &count_insere);
    ler_chaves_primarias(chaves_remove, &count_remove, 'r');
    ler_chaves_primarias(chaves_busca, &count_busca, 'b');

    for (int i = 0; i < count_remove; i++) {
        printf("ID: %s\n", chaves_remove[i].id_aluno);
        printf("Sigla: %s\n", chaves_remove[i].sigla_disc);
    }

    entrada = '0';

    while (entrada != 's') {
        printf("\nOpções: \n\n");
        printf("1 - Inserir aluno(i)\n");
        printf("2 - Remover aluno(r)\n");
        printf("3 - Buscar aluno(b)\n");
        printf("4 - Sair(s)\n");
        printf("\nDigite a opção desejada: ");

        entrada = getchar();
        getchar();

        switch (entrada) {
            case 'i':
                id = id_para_inserir();

                if (id < count_insere) {
                    inserir_aluno(alunos[id]);
                }
                else {
                    printf("Todos os alunos já foram inseridos\n");
                }
                break;
            case 'r':
                id = id_para_remover();

                if (id < count_remove) {
                    remover_hash(chaves_remove[id]);
                }
                else {
                    printf("Todos os alunos já foram removidos\n");
                }
                break;
            case 'b':
                id = id_para_buscar();

                if (id < count_busca) {
                    buscar_aluno(chaves_busca[id]);
                }
                else {
                    printf("Todos os alunos já foram buscados\n");
                }
                break;
            case 's':
                entrada = 's';
                break;
            default:
                printf("Opção inválida\n");
                break;
        }
    }

    return 0;
}
