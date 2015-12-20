BEGIN{
    FS="[(), ]"
}
NR==1{
    if ( $4 != 9  ) exit 1
    if ( $9 != 3  ) exit 1
    if ($14 != 3  ) exit 1
    if ($19 != 4  ) exit 1
    if ($24 != 5  ) exit 1
    if ($29 != 4.9) exit 1
    if ($34 != 8  ) exit 1
    if ($39 != 3.3) exit 1
    if ($44 != 2  ) exit 1
    if ($49 != 0.1) exit 1
}
NR==2{
    if ($1 != 5) exit 2
}
