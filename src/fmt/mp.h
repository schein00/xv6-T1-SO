6800 // See MultiProcessor Specification Version 1.[14]
6801 
6802 struct mp {             // floating pointer
6803   uchar signature[4];           // "_MP_"
6804   void *physaddr;               // phys addr of MP config table
6805   uchar length;                 // 1
6806   uchar specrev;                // [14]
6807   uchar checksum;               // all bytes must add up to 0
6808   uchar type;                   // MP system config type
6809   uchar imcrp;
6810   uchar reserved[3];
6811 };
6812 
6813 struct mpconf {         // configuration table header
6814   uchar signature[4];           // "PCMP"
6815   ushort length;                // total table length
6816   uchar version;                // [14]
6817   uchar checksum;               // all bytes must add up to 0
6818   uchar product[20];            // product id
6819   uint *oemtable;               // OEM table pointer
6820   ushort oemlength;             // OEM table length
6821   ushort entry;                 // entry count
6822   uint *lapicaddr;              // address of local APIC
6823   ushort xlength;               // extended table length
6824   uchar xchecksum;              // extended table checksum
6825   uchar reserved;
6826 };
6827 
6828 struct mpproc {         // processor table entry
6829   uchar type;                   // entry type (0)
6830   uchar apicid;                 // local APIC id
6831   uchar version;                // local APIC verison
6832   uchar flags;                  // CPU flags
6833     #define MPBOOT 0x02           // This proc is the bootstrap processor.
6834   uchar signature[4];           // CPU signature
6835   uint feature;                 // feature flags from CPUID instruction
6836   uchar reserved[8];
6837 };
6838 
6839 struct mpioapic {       // I/O APIC table entry
6840   uchar type;                   // entry type (2)
6841   uchar apicno;                 // I/O APIC id
6842   uchar version;                // I/O APIC version
6843   uchar flags;                  // I/O APIC flags
6844   uint *addr;                  // I/O APIC address
6845 };
6846 
6847 
6848 
6849 
6850 // Table entry types
6851 #define MPPROC    0x00  // One per processor
6852 #define MPBUS     0x01  // One per bus
6853 #define MPIOAPIC  0x02  // One per I/O APIC
6854 #define MPIOINTR  0x03  // One per bus interrupt source
6855 #define MPLINTR   0x04  // One per system interrupt source
6856 
6857 
6858 
6859 
6860 
6861 
6862 
6863 
6864 
6865 
6866 
6867 
6868 
6869 
6870 
6871 
6872 
6873 
6874 
6875 
6876 
6877 
6878 
6879 
6880 
6881 
6882 
6883 
6884 
6885 
6886 
6887 
6888 
6889 
6890 
6891 
6892 
6893 
6894 
6895 
6896 
6897 
6898 
6899 
6900 // Blank page.
6901 
6902 
6903 
6904 
6905 
6906 
6907 
6908 
6909 
6910 
6911 
6912 
6913 
6914 
6915 
6916 
6917 
6918 
6919 
6920 
6921 
6922 
6923 
6924 
6925 
6926 
6927 
6928 
6929 
6930 
6931 
6932 
6933 
6934 
6935 
6936 
6937 
6938 
6939 
6940 
6941 
6942 
6943 
6944 
6945 
6946 
6947 
6948 
6949 
