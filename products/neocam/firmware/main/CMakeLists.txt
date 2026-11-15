idf_component_register(
    SRCS "main.c" "../STConf/ConfAr/NeoCam_Composition.c"
    INCLUDE_DIRS "." "../STConf/ConfAr"
    REQUIRES BSW Rte ApplicationLayer log
)
