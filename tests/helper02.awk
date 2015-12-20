BEGIN{
    FS="[(), ]"
}
NR==1{
    if ( $4 != 4  ) exit 1
    if ( $9 != 5  ) exit 1
    if ($14 != 4.9) exit 1
    if ($19 != 8  ) exit 1
    if ($24 != 3.3) exit 1
    if ($29 != 2  ) exit 1
    if ($34 != 0.1) exit 1
}
NR==2{
    if ($1 != 4.9) exit 2
}
