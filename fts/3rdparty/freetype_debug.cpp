// see https://stackoverflow.com/questions/20190020/xcode-freetype-ld-symbols-not-found-for-architecture-x86-64#20190677
extern "C" {
int z_verbose = 0;

void z_error(/* should be const */char* message)
{
    /*log_somewhere_or_ignore(message);*/
}

}