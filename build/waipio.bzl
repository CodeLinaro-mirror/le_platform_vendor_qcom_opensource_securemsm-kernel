load(":securemsm_kernel.bzl", "define_consolidate_gki_modules", "define_vm_modules")

def define_waipio():
    define_consolidate_gki_modules(
        target = "waipio",
        modules = [
            "smcinvoke_dlkm",
            "tz_log_dlkm",
            "qseecom_dlkm",
            "hdcp_qseecom_dlkm",
            "qce50_dlkm",
            "qcedev-mod_dlkm",
            "qrng_dlkm",
            "qcrypto-msm_dlkm",
            "smmu_proxy_dlkm",
            "tmecom-intf_dlkm",
            "seccam_test_driver",
            "hdcp2p2_test",
            "si_core_test",
            "tornado_mod",
         ],
         extra_options = [
             "CONFIG_QCOM_SI_CORE_TEST",
             "CONFIG_QCOM_SMCINVOKE",
             "CONFIG_QSEECOM",
             "CONFIG_QCOM_SI_CORE",
         ],
     )

def define_waipio_vm():
    define_vm_modules(
        target = "waipio-tuivm",
        modules = [
            "smcinvoke_dlkm",
            "smmu_proxy_dlkm",
        ],
        extra_options = [
            "CONFIG_QCOM_SMCINVOKE",
            "CONFIG_QTI_SMMU_PROXY",
            "CONFIG_QCOM_SI_CORE",
            "CONFIG_ARCH_QTI_VM",
        ],
    )
