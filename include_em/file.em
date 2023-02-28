use inb

# Informs compiler that we have created a blank class,
# however we want the compiler to initialize the class for us,
# so below when 'createFileBuffer(...)' is called, it will initialize
# buffer with a compiler created object.
~interlink class FileBuffer mutable => {
    # We then specify what we want in the FileBuffer class.
    # this is useful for the user.
    # we mark the interlinked FileBuffer class as mutable, 
    # which means we allow other members / methods to be inserted
    # into this object, and the user will not be able to see them,
    # but they can use them.

    string path,
    int mode,
    method read(),
    method write(),
    method close(),
    method duplicate(string newpath)

}

method open(string path, string mode)
{
    FileBuffer buffer = null

    createFileBuffer(buffer, mode, )
    # Buffer is initialized


}