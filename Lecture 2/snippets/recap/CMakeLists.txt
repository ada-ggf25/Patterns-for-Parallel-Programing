# Recap — toolchain check + roofline pre-teach.
#
# These snippets ship in slides/recap.qmd via Quarto's include shortcode.

function(ppp_add_recap_test name)
  add_executable(test_recap_${name}
    tests/test_${name}.cpp
    ${name}.cpp
  )
  target_link_libraries(test_recap_${name}
    PRIVATE ppp_warnings OpenMP::OpenMP_CXX doctest
  )
  add_test(NAME recap_${name} COMMAND test_recap_${name})
  set_tests_properties(recap_${name} PROPERTIES LABELS "recap")
endfunction()

ppp_add_recap_test(roofline_calc_demo)
