idf_component_register(
    SRCS "ASW/CamHmi/CamHmi.c" "ASW/CamDiag/CamDiag.c" "DE/CamDev/CamDev.c" "ASV/CamSupv/CamSupv.c"
    INCLUDE_DIRS "ASW/CamHmi" "ASW/CamDiag" "DE/CamDev" "ASV/CamSupv"
    REQUIRES Rte
)
