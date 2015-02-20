#include <libxml/catalog.h>


static int bam_catalog_loaded = 0;


int bam_xmlLoadCatalog() {
        char catalog[] = BAM_CATALOG;
        int rv;

        if (bam_catalog_loaded)
                return 1;

        if (xmlLoadCatalog(catalog) != 0) {
                fprintf(stderr,"Could not load %s\n",catalog);
                rv=0;
        }
        else
                rv=1;

        bam_catalog_loaded = 1;

        return rv;
}
