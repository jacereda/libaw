int main(int argc, char ** argv) {
        extern int fakemain(argc, argv);
        return fakemain(argc, argv);
}
