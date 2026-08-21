#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

PROG="./codexion"
VALGRIND=""

# ---- Normal / stress test cases : "label|args|expected" ----
# expected: SUCCESS   -> no coder should burn out
#           BURNOUT   -> at least one coder is expected to burn out
#           UNKNOWN   -> depends on timing / not guaranteed either way
TESTS=(
    "Basic FIFO|4 800 200 200 200 5 10 fifo|SUCCESS"
    "Basic EDF|4 800 200 200 200 5 10 edf|SUCCESS"
    "Success FIFO (long)|10 10000 100 100 100 5 50 fifo|SUCCESS"
    "Large EDF|20 5000 500 500 500 10 100 edf|SUCCESS"
    "Low cooldown|5 2000 100 100 100 20 1 fifo|SUCCESS"
    "Long actions|3 10000 2000 2000 2000 2 100 fifo|SUCCESS"
    "Big test (100 coders)|100 10000 66 24 87 10 10 fifo|SUCCESS"
    "Tight timing FIFO|3 1000 600 10 10 5 100 fifo|UNKNOWN"
    "Tight timing EDF|3 1000 600 10 10 5 100 edf|UNKNOWN"
    "One coder FIFO|1 1000 200 200 200 5 50 fifo|SUCCESS"
    "One coder EDF|1 1000 200 200 200 5 50 edf|SUCCESS"
    "Zero compiles required|5 1000 200 200 200 0 10 fifo|SUCCESS"
    "Immediate burnout|2 1 200 200 200 5 10 fifo|BURNOUT"
    "Cooldown hell|2 1000 100 100 100 5 2000 fifo|BURNOUT"
    "Max coders|300 10000 100 100 100 5 10 edf|UNKNOWN"
    "Too many coders|999 1000 200 200 200 5 50 fifo|BURNOUT"
)

# ---- Error / invalid argument cases : "label|args" ----
ERROR_TESTS=(
    "Non-numeric coder count|banana 200 300 400 500 5 10 fifo"
    "Non-numeric burnout time|10 banana 300 400 500 5 10 fifo"
    "Non-numeric compile time|10 200 banana 400 500 5 10 fifo"
    "Non-numeric debug time|10 200 300 banana 500 5 10 fifo"
    "Non-numeric refactor time|10 200 300 400 banana 5 10 fifo"
    "Non-numeric compiles required|10 200 300 400 500 banana 10 fifo"
    "Non-numeric cooldown|10 200 300 400 500 5 banana fifo"
    "Invalid scheduler name|10 200 300 400 500 5 10 banana"
    "Negative value|10 200 300 -400 500 5 10 edf"
    "Wrong argument count (too many)|too 10 200 300 400 500 5 10 edf"
    "Wrong argument count (too few)|10 200 300 400 500 5 10"
    "Zero coders|0 200 300 400 500 5 10 fifo"
)

expected_color() {
    case "$1" in
        SUCCESS) echo -e "${GREEN}$1${NC}" ;;
        BURNOUT) echo -e "${RED}$1${NC}" ;;
        *)       echo -e "${YELLOW}$1${NC}" ;;
    esac
}

run_test() {
    local label="$1"
    local args="$2"
    local expected="$3"
    echo -e "${CYAN}--- $label ---${NC}"
    if [ -n "$expected" ]; then
        echo -e "Expected outcome: $(expected_color "$expected")"
    fi
    echo -e "${YELLOW}$PROG $args${NC}\n"
    $VALGRIND $PROG $args
    local rc=$?
    echo ""
    if [ $rc -eq 0 ]; then
        echo -e "${GREEN}Exit code: $rc${NC}"
    else
        echo -e "${RED}Exit code: $rc${NC}"
    fi
}

select_valgrind_mode() {
    echo -e "${CYAN}Execution mode:${NC}"
    echo "  0) Normal"
    echo "  1) Valgrind (memcheck)"
    echo "  2) Valgrind (helgrind)"
    read -rp "Choice: " mode
    case "$mode" in
        1) VALGRIND="valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes" ;;
        2) VALGRIND="valgrind --tool=helgrind" ;;
        *) VALGRIND="" ;;
    esac
}

print_mode_banner() {
    if [ -n "$VALGRIND" ]; then
        echo -e "${YELLOW}Mode: $VALGRIND${NC}"
        echo ""
    fi
}

pause() {
    echo ""
    read -rp "Press Enter to run again, or 'b' to go back: " again
}

# ---- Normal tests submenu ----
show_normal_menu() {
    clear
    echo -e "${CYAN}=== Codexion Tester - Normal / Stress Tests ===${NC}"
    print_mode_banner
    echo -e "Legend: ${GREEN}SUCCESS${NC}=no burnout expected  ${RED}BURNOUT${NC}=burnout expected  ${YELLOW}UNKNOWN${NC}=timing-dependent"
    echo ""
    local i=1
    for entry in "${TESTS[@]}"; do
        IFS='|' read -r label args expected <<< "$entry"
        tag=$(expected_color "$expected")
        printf "  %2d) %-28s [%b]\n" "$i" "$label" "$tag"
        i=$((i+1))
    done
    echo ""
    echo "   b) Back to main menu"
    echo ""
    read -rp "Choice: " choice
}

normal_tests_loop() {
    while true; do
        show_normal_menu
        if [[ "$choice" == "b" ]]; then
            break
        elif [[ "$choice" =~ ^[0-9]+$ ]] && [ "$choice" -ge 1 ] && [ "$choice" -le "${#TESTS[@]}" ]; then
            entry="${TESTS[$((choice-1))]}"
            IFS='|' read -r label args expected <<< "$entry"
            while true; do
                echo ""
                run_test "$label" "$args" "$expected"
                pause
                if [[ "$again" == "b" ]]; then
                    break
                fi
            done
        else
            echo -e "${RED}Invalid choice.${NC}"
            pause
        fi
    done
}

# ---- Error tests submenu ----
show_error_menu() {
    clear
    echo -e "${CYAN}=== Codexion Tester - Error / Invalid Argument Tests ===${NC}"
    print_mode_banner
    local i=1
    for entry in "${ERROR_TESTS[@]}"; do
        label="${entry%%|*}"
        printf "  %2d) %s\n" "$i" "$label"
        i=$((i+1))
    done
    echo ""
    echo "   a) Run all error tests"
    echo "   b) Back to main menu"
    echo ""
    read -rp "Choice: " choice
}

error_tests_loop() {
    while true; do
        show_error_menu
        if [[ "$choice" == "b" ]]; then
            break
        elif [[ "$choice" == "a" ]]; then
            echo ""
            for entry in "${ERROR_TESTS[@]}"; do
                label="${entry%%|*}"
                args="${entry#*|}"
                run_test "$label" "$args"
                echo ""
            done
            pause
        elif [[ "$choice" =~ ^[0-9]+$ ]] && [ "$choice" -ge 1 ] && [ "$choice" -le "${#ERROR_TESTS[@]}" ]; then
            entry="${ERROR_TESTS[$((choice-1))]}"
            label="${entry%%|*}"
            args="${entry#*|}"
            while true; do
                echo ""
                run_test "$label" "$args"
                pause
                if [[ "$again" == "b" ]]; then
                    break
                fi
            done
        else
            echo -e "${RED}Invalid choice.${NC}"
            pause
        fi
    done
}

# ---- Main menu ----
show_main_menu() {
    clear
    echo -e "${CYAN}=== Codexion Tester ===${NC}"
    print_mode_banner
    echo "  1) Normal / stress tests"
    echo "  2) Error / invalid argument tests"
    echo "  v) Change execution mode (normal/valgrind/helgrind)"
    echo "  q) Quit"
    echo ""
    read -rp "Choice: " choice
}

main_loop() {
    while true; do
        show_main_menu
        case "$choice" in
            1) normal_tests_loop ;;
            2) error_tests_loop ;;
            v) select_valgrind_mode ;;
            q) break ;;
            *)
                echo -e "${RED}Invalid choice.${NC}"
                pause
                ;;
        esac
    done
}

if [ ! -x "$PROG" ]; then
    echo -e "${RED}$PROG not found or not executable (run 'make' first).${NC}"
    exit 1
fi

main_loop