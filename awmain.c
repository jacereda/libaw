int main(int argc, char ** argv) {
        extern int fakemain(int, char **);
        return fakemain(argc, argv);
}
