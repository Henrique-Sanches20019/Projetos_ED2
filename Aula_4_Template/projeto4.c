// Projeto Aula 04 - Henrique Sanches e Renan ALves

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

struct remove {
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

void ler_alunos_remove(struct remove *alunos, int *count) { // Lê os alunos a serem removidos
    FILE *fd;
    *count = 0;

    if ((fd = fopen("remove.bin", "rb")) == NULL) {
        printf("Nao foi possivel abrir o arquivo\n");
        return;
    }

    while (fread(&alunos[*count], sizeof(struct remove), 1, fd) == 1) {
        (*count)++;
    }

    fclose(fd);
}

int id_para_inserir() { // Guarda a posição do próximo registro a ser inserido
    FILE *fd;
    int arquivoExiste, id_inserir, id_remover, id_inserir_novo;

    arquivoExiste = arquivo_existe("arquivoAuxiliar.bin");

    if (!arquivoExiste) {
        if ((fd = fopen("arquivoAuxiliar.bin","w+b")) == NULL) {
            printf("Nao foi possivel abrir o arquivo");
            return -1;
        }

        id_inserir = 1;
        id_remover = 0;
        fwrite(&id_inserir, sizeof(int), 1, fd);
        fwrite(&id_remover, sizeof(int), 1, fd);
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

void ler_campo_var(FILE *fd, char campo[]) { // Lê campos de tamanho variável
    int i = 0;
    char ch;

    fread(&ch, sizeof(char), 1, fd);

    while(ch != '#') {
        campo[i] = ch;
        i++;
        
        fread(&ch, sizeof(char), 1, fd);
    }
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

void inserir_aluno(struct aluno aluno) {
    FILE *fd;
    int arquivoExiste, tamRegistro;

    arquivoExiste = arquivo_existe("lista_historicos.bin");

    tamRegistro = 4 + 4 + strlen(aluno.nome_aluno) + 1 + strlen(aluno.nome_disc) + 1 + sizeof(float) + sizeof(float) + 5;

    if (!arquivoExiste) {
        if ((fd = fopen("lista_historicos.bin","w+b")) == NULL) {
            printf("Nao foi possivel abrir o arquivo");
            return;
        }

        int dispo = -1; // Cria a lista de disponíveis como vazia
        fwrite(&dispo, sizeof(int), 1, fd); // Escreve o header

        escrever_aluno(aluno, tamRegistro, fd);
        fclose(fd);
    }
    else {
        if ((fd = fopen("lista_historicos.bin","r+b")) == NULL) {
            printf("Nao foi possivel abrir o arquivo");
            return;
        }

        int offset, espaco_disp, ant_offset, prox_offset;

        ant_offset = 0;
        fseek(fd, 0, SEEK_SET);
        fread(&offset, sizeof(int), 1, fd); // Lê o header

        while (offset != -1) {
            fseek(fd, offset, SEEK_SET); // Vai pro primeiro registro da lista de disponíveis
            fread(&espaco_disp, sizeof(int), 1, fd); // Lê tamanho do registro

            fseek(fd, sizeof(char), SEEK_CUR);
            fread(&prox_offset, sizeof(int), 1, fd); // Lê o offset do próximo elemento da lista de disponíveis

            if (espaco_disp >= tamRegistro) { // Verifica se o registro cabe em algum espaço na lista de disponíveis
                if (ant_offset != 0) {
                    fseek(fd, ant_offset + sizeof(int) + sizeof(char), SEEK_SET);
                    fwrite(&prox_offset, sizeof(int), 1, fd);
                }
                else {
                    fseek(fd, 0, SEEK_SET);
                    fwrite(&prox_offset, sizeof(int), 1, fd);
                }

                fseek(fd, offset, SEEK_SET);
                escrever_aluno(aluno, espaco_disp, fd);
                fclose(fd);

                return;
            }
            else {
                ant_offset = offset;
                offset = prox_offset;
            }
        }

        fseek(fd, 0, SEEK_END);
        escrever_aluno(aluno, tamRegistro, fd); // Grava o resgistro no final do arquivo
        fclose(fd);
    }
}

void remover_aluno(struct remove aluno) {
    FILE *fd;
    int tamRegistro, offset, listaInicio;
    char id_aluno[4], sigla_disc[4], marcador = '*';

    if ((fd = fopen("lista_historicos.bin","r+b")) == NULL) {
        printf("Nao foi possivel abrir o arquivo");
        return;
    }

    fread(&listaInicio, sizeof(int), 1, fd); // Lê o header

    while (fread(&tamRegistro, sizeof(int), 1, fd) == 1) { // Percorre o arquivo
        fread(&id_aluno, sizeof(id_aluno), 1, fd);

        fseek(fd, sizeof(char), SEEK_CUR);
        fread(&sigla_disc, sizeof(sigla_disc), 1, fd);

        if (strncmp(id_aluno, aluno.id_aluno, 4) == 0 && strncmp(sigla_disc, aluno.sigla_disc, 4) == 0) { // Verifica se o registro é o que deve ser removido            
            fseek(fd, -(sizeof(id_aluno) + sizeof(sigla_disc) + sizeof(char) + sizeof(tamRegistro)), SEEK_CUR);
            offset = ftell(fd);

            fseek(fd, sizeof(tamRegistro), SEEK_CUR);
            fwrite(&marcador, sizeof(marcador), 1, fd);
            fwrite(&listaInicio, sizeof(listaInicio), 1, fd);

            fseek(fd, 0, SEEK_SET);
            fwrite(&offset, sizeof(offset), 1, fd);

            fclose(fd);

            return;
        }

        fseek(fd, tamRegistro - sizeof(id_aluno) - sizeof(sigla_disc) - sizeof(char), SEEK_CUR);
    }

    printf("Aluno não encontrado\n");
    fclose(fd);
}

void compactar_arquivo() {
    FILE *fd, *fdCompactado;
    int tamRegistro, tamReal, offset;
    char marcador, campo_var[50];
    struct aluno buffer;

    fd = fopen("lista_historicos.bin","r+b");
    fdCompactado = fopen("lista_historicos_compactado.bin","w+b");
    
    if (!fd || !fdCompactado) {
        printf("Nao foi possivel abrir os arquivos");
        return;
    }

    int dispo = -1; // Cria a lista de disponíveis como vazia
    fwrite(&dispo, sizeof(int), 1, fdCompactado); // Escreve o header

    fseek(fd, sizeof(int), SEEK_SET);

    while (fread(&tamRegistro, sizeof(int), 1, fd) == 1) { // Percorre o arquivo
        offset = ftell(fd);
        fread(&marcador, sizeof(char), 1, fd);
        fseek(fd, -(sizeof(char)), SEEK_CUR);

        tamReal = 0;

        if (marcador != '*') { // Grava o registro no arquivo compactado
            fread(&buffer.id_aluno, sizeof(buffer.id_aluno), 1, fd);
            fseek(fd, sizeof(char), SEEK_CUR);
            tamReal += sizeof(buffer.id_aluno) + sizeof(char);

            fread(&buffer.sigla_disc, sizeof(buffer.sigla_disc), 1, fd);
            fseek(fd, sizeof(char), SEEK_CUR);
            tamReal += sizeof(buffer.sigla_disc) + sizeof(char);

            ler_campo_var(fd, campo_var);
            strcpy(buffer.nome_aluno, campo_var);
            tamReal +=  sizeof(campo_var) + sizeof(char);

            ler_campo_var(fd, campo_var);
            strcpy(buffer.nome_disc, campo_var);
            tamReal +=  sizeof(campo_var) + sizeof(char);

            fread(&buffer.media, sizeof(buffer.media), 1, fd);
            fseek(fd, sizeof(char), SEEK_CUR);
            tamReal += sizeof(buffer.media) + sizeof(char);

            fread(&buffer.freq, sizeof(buffer.freq), 1, fd);
            tamReal += sizeof(buffer.freq);

            escrever_aluno(buffer, tamReal, fdCompactado);
        }

        fseek(fd, offset + tamRegistro, SEEK_SET);
    }

    fclose(fd);
    fclose(fdCompactado);

    remove("lista_historicos.bin");
}

int main() {
    struct aluno alunos[100];
    struct remove alunosRemovidos[100];
    int count_insere, count_remove, id, i = 0, j = 0;
    char entrada;

    ler_alunos_insere(alunos, &count_insere);
    ler_alunos_remove(alunosRemovidos, &count_remove);

    entrada = '0';

    while (entrada != 's') {
        printf("\nOpções: \n\n");
        printf("1 - Inserir aluno(i)\n");
        printf("2 - Remover aluno(r)\n");
        printf("3 - Compactar arquivo(c)\n");
        printf("4 - Sair(s)\n");
        printf("\nDigite a opção desejada: ");

        entrada = getchar();
        getchar();

        switch (entrada) {
            case 'i':
                if (i < (count_insere - 1)) {
                    id = id_para_inserir();
                    inserir_aluno(alunos[id]);
                    i = id;
                }
                else {
                    printf("Todos os alunos já foram inseridos\n");
                }
                break;
            case 'r':
                if (j < (count_remove - 1)) {
                    id = id_para_remover();
                    remover_aluno(alunosRemovidos[id]);
                    j = id;
                }
                else {
                    printf("Todos os alunos já foram removidos\n");
                }
                break;
            case 'c':
                compactar_arquivo();
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
