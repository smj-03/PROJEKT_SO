//
// Created by Szymon on 1/19/2025.
//
#include <utilities.h>

#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"

/**
 *  Funkcja do logowania wiadomości do konsoli.
 *
 * @param _process_name Nazwa procesu, który wysyła wiadomość.
 * @param _format Format wiadomości.
 * @param ... Elementy zawarte w wiadomości.
 * @return Długość wiadomości.
 */
int log_message(const char *_process_name, const char *_format, ...) {
    const time_t now = time(NULL);
    const struct tm *local_time = localtime(&now);

    fprintf(stdout, "[%02d:%02d:%02d]",
            local_time->tm_hour,
            local_time->tm_min,
            local_time->tm_sec);

    fprintf(stdout, "[%s%s%s] ",
            ANSI_COLOR_GREEN,
            _process_name,
            ANSI_COLOR_RESET);

    va_list args;
    va_start(args, _format);

    const int result = vfprintf(stdout, _format, args);

    va_end(args);

    fflush(stdout);

    return result;
}

/**
 * Funkcja do logowania ostrzeżeń do konsoli.
 *
 * @param _process_name Nazwa procesu, który wysyła wiadomość.
 * @param _format Format wiadomości.
 * @param ... Elementy zawarte w wiadomości.
 * @return Długość wiadomości.
 */
int log_warning(const char *_process_name, const char *_format, ...) {
    const time_t now = time(NULL);
    const struct tm *local_time = localtime(&now);

    fprintf(stdout, "[%02d:%02d:%02d]",
            local_time->tm_hour,
            local_time->tm_min,
            local_time->tm_sec);

    fprintf(stdout, "[%s%s%s] ",
            ANSI_COLOR_YELLOW,
            _process_name,
            ANSI_COLOR_RESET);

    va_list args;
    va_start(args, _format);

    const int result = vfprintf(stdout, _format, args);

    va_end(args);

    fflush(stdout);

    return result;
}

/**
 * Funkcja wypisująca błąd oraz kończąca proces.
 *
 * @param _process_name Nazwa procesu, który wysyła wiadomość.
 * @param _format Format wiadomości.
 * @param ... Elementy zawarte w wiadomości.
 */
void throw_error(const char *_process_name, const char *_format, ...) {
    const time_t now = time(NULL);
    const struct tm *local_time = localtime(&now);

    fprintf(stderr, "[%s%02d:%02d:%02d%s]",
            ANSI_COLOR_RED,
            local_time->tm_hour,
            local_time->tm_min,
            local_time->tm_sec,
            ANSI_COLOR_RESET);

    fprintf(stderr, "[%s%s%s]",
            ANSI_COLOR_RED,
            _process_name,
            ANSI_COLOR_RESET);

    fprintf(stderr, "[%sERROR %d%s] ",
            ANSI_COLOR_RED,
            errno,
            ANSI_COLOR_RESET);

    va_list args;
    va_start(args, _format);

    vfprintf(stderr, _format, args);

    va_end(args);

    if (errno != 0) {
        fprintf(stderr, ": %s\n", strerror(errno));
    } else fprintf(stderr, "\n");


    fflush(stderr);

    exit(1);
}

/**
 * Funkcja do pobierania ścieżki procesu.
 *
 * @param path Ścieżka procesu.
 * @param process_name Nazwa procesu.
 */
void get_process_path(char *path, const char *process_name) {
    path[0] = '\0';
    strcat(path, "./");
    strcat(path, process_name);
}

/**
 * Funkcja generująca pseudo losową liczbę.
 *
 * @param min Dolny zakres losowania.
 * @param max Górny zakres losowania.
 * @return Pseudo losowa liczba.
 */
int get_random_number(const int min, const int max) {
    return rand() % (max - min + 1) + min;
}

/**
 * Funkcja do alokacji semafora.
 *
 * @param key Klucz semafora.
 * @param number Liczba semaforów.
 * @param flags Opcjonalne flagi.
 * @return Identyfikator semafora.
 */
int sem_alloc(const key_t key, const int number, const int flags) {
    return semget(key, number, flags);
}

/**
 * Funkcja do inicjalizacji semafora.
 *
 * @param sem_id Identyfukator semafora.
 * @param number Liczba semaforów.
 * @param value Wartość początkowa.
 * @return Wartość zwracana funkcji semctl.
 */
int sem_init(const int sem_id, const int number, const int value) {
    return semctl(sem_id, number, SETVAL, value);
}

/**
 * Funkcja do zwiększenia wartości semafora.
 *
 * @param sem_id Identyfikator semafora.
 * @param number Numer semafora.
 * @return Wartość zwracana funkcji semop.
 */
int sem_post(const int sem_id, const int number) {
    struct sembuf operations[1];
    operations[0].sem_num = number;
    operations[0].sem_op = 1;

    return semop(sem_id, operations, 1);
}

/**
 * Funkcja do zmniejszenia wartości semafora.
 *
 * @param sem_id Identyfikator semafora.
 * @param number Numer semafora.
 * @param flags Opcjonalne flagi.
 * @return Wartość zwracana funkcji semop.
 */
int sem_wait(const int sem_id, const int number, const int flags) {
    struct sembuf operations[1];
    operations[0].sem_num = number;
    operations[0].sem_op = -1;
    operations[0].sem_flg = 0 | flags;

    return semop(sem_id, operations, 1);
}

/**
 * Funkcja do oczekiwania na semaforze bez zmiany wartości.
 *
 * @param sem_id Identyfikator semafora.
 * @param number Numer semafora.
 * @param flags Opcjonalne flagi.
 * @return Wartość zwracana funkcji semop.
 */
int sem_wait_no_op(const int sem_id, const int number, const int flags) {
    struct sembuf operations[1];
    operations[0].sem_num = number;
    operations[0].sem_op = 0;
    operations[0].sem_flg = 0 | flags;

    return semop(sem_id, operations, 1);
}

/**
 * Funkcja do usuwania semafora.
 *
 * @param sem_id Identyfikator semafora.
 * @param number Numer semafora.
 * @return Wartość zwracana funkcji semctl.
 */
int sem_destroy(const int sem_id, const int number) {
    return semctl(sem_id, number, IPC_RMID, NULL);
}

/**
 * Funkcja do alokacji bloku pamięci współdzielonej.
 *
 * @param key Klucz bloku pamięci współdzielonej.
 * @param size Rozmiar bloku.
 * @param flags Opcjonalne flagi.
 * @return Identyfikator bloku pamięci współdzielonej.
 */
int shared_block_alloc(const key_t key, const size_t size, const int flags) {
    return shmget(key, size, flags);
}

/**
 * Funkcja do dołączenia bloku pamięci współdzielonej.
 *
 * @param key Klucz bloku pamięci współdzielonej.
 * @param size Rozmiar bloku.
 * @return Wskaźnik na blok pamięci współdzielonej.
 */
void *shared_block_attach(const key_t key, int size) {
    const int shared_block_id = shared_block_alloc(key, size, IPC_GET);
    if (shared_block_id == IPC_ERROR) return NULL;

    char *result = shmat(shared_block_id, NULL, 0);
    if (result == (char *) IPC_ERROR) return NULL;

    return result;
}

/**
 * Funkcja do odłączenia bloku pamięci współdzielonej.
 *
 * @param block Wskaźnik na blok pamięci współdzielonej.
 * @return Wartość zwracana funkcji shmdt.
 */
int shared_block_detach(const void *block) {
    return shmdt(block);
}

/**
 * Funkcja do usuwania bloku pamięci współdzielonej.
 *
 * @param key Klucz bloku pamięci współdzielonej.
 * @return Wartość zwracana funkcji shmctl.
 */
int shared_block_destroy(const key_t key) {
    const int shared_block_id = shared_block_alloc(key, 0, IPC_GET);
    if (shared_block_id == IPC_ERROR) return -1;

    return shmctl(shared_block_id, IPC_RMID, NULL);
}

/**
 * Funkcja do alokacji kolejki komunikatów.
 *
 * @param key Klucz kolejki komunikatów.
 * @param flags Opcjonalne flagi.
 * @return Identyfikator kolejki komunikatów.
 */
int message_queue_alloc(const key_t key, const int flags) {
    return msgget(key, flags);
}

/**
 * Funkcja do wysłania komunikatu do kolejki komunikatów.
 *
 * @param msg_id Identyfikator kolejki komunikatów.
 * @param message Komunikat.
 * @return Wartość zwracana funkcji msgsnd.
 */
int message_queue_send(const int msg_id, const struct message *message) {
    return msgsnd(msg_id, message, sizeof(message->mvalue), 0);
}

/**
 * Funkcja do odbierania komunikatu z kolejki komunikatów.
 *
 * @param msg_id Identyfikator kolejki komunikatów.
 * @param message Komunikat.
 * @param mtype Typ komunikatu.
 * @param flags Opcjonalne flagi.
 * @return Wartość zwracana funkcji msgrcv.
 */
ssize_t message_queue_receive(const int msg_id, struct message *message, const long int mtype, const int flags) {
    return msgrcv(msg_id, message, sizeof(message->mvalue), mtype, flags);
}

/**
 * Funkcja do usuwania kolejki komunikatów.
 *
 * @param msg_id Identyfikator kolejki komunikatów.
 * @return Wartość zwracana funkcji msgctl.
 */
int message_queue_destroy(const int msg_id) {
    return msgctl(msg_id, IPC_RMID, NULL);
}

/**
 * Funkcja do ustawienia obsługi sygnału.
 *
 * @param signal Numer sygnału.
 * @param handler Funkcja obsługi sygnału.
 * @return Wartość zwracana funkcji sigaction.
 */
int setup_signal_handler(int signal, void (*handler)(int)) {
    struct sigaction sa;
    sa.sa_handler = handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    if (sigaction(signal, &sa, NULL) == -1) return -1;
    return 0;
}

/**
 * Funkcja do oczekiwania na sygnał.
 *
 * @param signal Numer sygnału.
 * @return Numer odebranego sygnału.
 */
int wait_for_signal(const int signal) {
    sigset_t new_set, old_set;
    int sig;

    sigemptyset(&new_set);
    sigaddset(&new_set, signal);

    if (sigprocmask(SIG_BLOCK, &new_set, &old_set) == -1) return -1;

    if (sigwait(&new_set, &sig) == -1) {
        sigprocmask(SIG_SETMASK, &old_set, NULL);
        return -1;
    }

    if (sigprocmask(SIG_SETMASK, &old_set, NULL) == -1) return -1;

    return sig;
}

/**
 * Funkcja dodatająca wartość do stosu pasażerów.
 *
 * @param stack Stos pasażerów.
 * @param value Wartość.
 * @return 1 jeśli wartość została dodana, -1 w przeciwnym wypadku.
 */
int push(struct passenger_stack *stack, const int value) {
    if (stack->top < TRAIN_MAX_CAPACITY)
        stack->data[stack->top++] = value;
    else return -1;
    return 1;
}

/**
 * Funkcja zdejmująca wartość ze stosu pasażerów.
 *
 * @param stack Stos pasażerów.
 * @return Zdjęta wartość.
 */
int pop(struct passenger_stack *stack) {
    if (stack->top > 0)
        return stack->data[--stack->top];
    return -1;
}

