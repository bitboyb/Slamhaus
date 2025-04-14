:section[align:center](#element-example)

# Element Examples

## Scripts

?script[](#my-script)

    function helloFunction() {
        document.getElementById("test").innerHTML = "Hello From Script!";
    }

?/script

?button[type:function text:"press me!" value=helloFunction](#test-button)

:/section

:section[align:center](#test)

Nothing yet...

:/section

:section[align:center](#button-example)

?button[type:url text:"go to google!" value:https://www.google.com](#google-button)

:/section

:section[align:center](#form-example)

?form[action:"mailto:test@email.com" method="post"](#email-form)

Full Name

?text[](#form-name)

Protein Option

?radio[value:veggie](#radio-veggie)
?radio[value:beef](#radio-beef)

Extras

?check[value:cheese](#check-cheese)
?check[value:pickles](#check-pickles)
?check[value:sauce](#check-sauce)

?button[type:form text:"Reset Form" value:"Reset"](#reset-button)
?button[type:form text:"Submit!" value:"Submit"](#submit-button)

?/form

:/section