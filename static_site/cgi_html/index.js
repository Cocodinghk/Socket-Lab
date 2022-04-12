let URL = "127.0.0.0:9999";

$(".button").on('click', () => {
    let name = $(".name").val();
    let password = $(".password").val();

    console.log(name, password)

    let params = {
        name: name,
        password: password
    }

    fetch(URL, {
        method: 'post',
        body: JSON.stringify(params)
    }).then(res => {
        if (res.status == 20) {
            console.log("Success");
            console.log(res.json());
        }
        else console.log("Failure", res.status);
    })
        .catch(err => {
            console.log(err)
        })
})
